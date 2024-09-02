#include <format>
#include <vector>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

namespace ct = clang::tooling;

class AccessSpecifierVisitor : public clang::RecursiveASTVisitor<AccessSpecifierVisitor> {
public:
    explicit AccessSpecifierVisitor(clang::ASTContext& Context) : Context(&Context) {}

    bool VisitCXXRecordDecl(clang::CXXRecordDecl *ClassDecl) {
        // Only handle the definition of the class (not just a declaration)
        if (!ClassDecl->isThisDeclarationADefinition()) {
            return true;
        }

		//clang::SourceManager& sm = Context->getSourceManager();
        //const auto& fileId = sm.getFileID(ClassDecl->getLocation());

        unsigned PublicCount = 0, ProtectedCount = 0, PrivateCount = 0;
		//if(fileId != sm.getMainFileID()) {return true;} //只处理主文件, but still return true, so that it can continue to traverse other classes
        // Traverse the class members
        for (const auto *D : ClassDecl->decls()) {
            if (const auto *AccessSpec = dyn_cast<clang::AccessSpecDecl>(D)) {
                switch (AccessSpec->getAccess()) {
                case clang::AS_public:
                    ++PublicCount;
                    break;
                case clang::AS_protected:
                    ++ProtectedCount;
                    break;
                case clang::AS_private:
                    ++PrivateCount;
                    break;
                default:
                    break;
                }
            }
        }

		llvm::outs() << std::format("Class: {}\nPublic: {}\nProtected: {}\nPrivate: {}\n", 
		ClassDecl->getNameAsString(), PublicCount, ProtectedCount, PrivateCount);

        return true; // 继续遍历其他类
    }

private:
    clang::ASTContext *Context;
};

//核心是上面的部分，下面的部分是重复的

class FindAccessSpecAstConsumer : public clang::ASTConsumer {
public:
	void HandleTranslationUnit(clang::ASTContext& astContext) final {
		clang::TranslationUnitDecl* tuDecl =
		  astContext.getTranslationUnitDecl();
		AccessSpecifierVisitor astVisitor(astContext);
		astVisitor.TraverseDecl(tuDecl);
	}
};

class MyFrontendAction : public clang::ASTFrontendAction {
public:
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance& compInstance, clang::StringRef) final {
		return std::unique_ptr<clang::ASTConsumer>{new FindAccessSpecAstConsumer};
	}
};

static llvm::cl::OptionCategory toolOptions("Tool Options");

int main(int argc, char** argv) {
	auto expectedOptionsParser = ct::CommonOptionsParser::create(argc,
	  const_cast<const char**>(argv), toolOptions);
	if (!expectedOptionsParser) {
		llvm::errs() << std::format("Unable to create option parser ({}).\n",
		  llvm::toString(expectedOptionsParser.takeError()));
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = *expectedOptionsParser;
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	int status = tool.run(
	  ct::newFrontendActionFactory<MyFrontendAction>().get());
	if (status) {llvm::errs() << "error detected\n";}
	return !status ? 0 : 1;
}

