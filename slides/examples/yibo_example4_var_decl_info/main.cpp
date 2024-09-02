#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::tooling;

class VarDeclVisitor : public RecursiveASTVisitor<VarDeclVisitor> {
public:
    explicit VarDeclVisitor(ASTContext *Context)
        : Context(Context) {}

    bool VisitVarDecl(VarDecl *VD) {
        //这里应该用source manager来获取文件名，行号，列号以及源代码文本（都行似乎）
        FullSourceLoc FullLocation = Context->getFullLoc(VD->getLocation());
        clang::SourceManager& sm = Context->getSourceManager();
        const auto& fileId = sm.getFileID(VD->getLocation());
        if (FullLocation.isValid() && fileId == sm.getMainFileID()) { 
            llvm::outs() << "File: " << sm.getFilename(VD->getLocation()) << "\n";
            llvm::outs() << "Line: " << FullLocation.getSpellingLineNumber()
                         << ", Column: " << FullLocation.getSpellingColumnNumber() << "\n";
            llvm::outs() << "Variable declaration: " << VD->getNameAsString() << "\n";
            llvm::outs() << "Source code text: " << VD->getType().getAsString() << "\n";//这里有问题

            // Check if the variable has global or local storage
            if (VD->hasGlobalStorage()) {
                llvm::outs() << "Storage: Global\n";
            } else {
                llvm::outs() << "Storage: Local\n";
            }

            // Check if the variable is static or non-static
            if (VD->isStaticLocal() || VD->isStaticDataMember() || ((VD->hasGlobalStorage() && VD->getFormalLinkage() == clang::Linkage::Internal))) {
                llvm::outs() << "Static: Yes\n";
            } else {
                llvm::outs() << "Static: No\n";
            }

            // Check if the variable is a template parameter
            if (isa<TemplateTypeParmDecl>(VD)) {
                llvm::outs() << "Template Parameter: Yes\n";
            } else {
                llvm::outs() << "Template Parameter: No\n";
            }

            // Check if the variable is a function parameter
            if (VD->isFunctionOrMethodVarDecl()) {// isa<ParmVarDecl>(VD)这么写也可以
                llvm::outs() << "Function Parameter: Yes\n";
            } else {
                llvm::outs() << "Function Parameter: No\n";
            }

            llvm::outs() << "-----------------------------------\n";
        }
        return true;
    }

private:
    ASTContext *Context;
};

class VarDeclASTConsumer : public clang::ASTConsumer {
public:
    explicit VarDeclASTConsumer(ASTContext *Context)
        : Visitor(Context) {}

    void HandleTranslationUnit(clang::ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    VarDeclVisitor Visitor;
};

class VarDeclFrontendAction : public clang::ASTFrontendAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &CI, llvm::StringRef file) override {
        return std::make_unique<VarDeclASTConsumer>(&CI.getASTContext());
    }
};

static llvm::cl::OptionCategory toolOptions("Tool Options");

int main(int argc, const char **argv) {
	auto expectedOptionsParser = CommonOptionsParser::create(argc,
	  const_cast<const char**>(argv), toolOptions);
	if (!expectedOptionsParser) {
		llvm::errs() << std::format("Unable to create option parser ({}).\n",
		  llvm::toString(expectedOptionsParser.takeError()));
		return 1;
	}
	CommonOptionsParser& optionsParser = *expectedOptionsParser;
	ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());

    return tool.run(newFrontendActionFactory<VarDeclFrontendAction>().get());
}

