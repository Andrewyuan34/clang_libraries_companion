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

struct AttributeCounter {
    std::map<std::string, int> standardAttributes;
    int nonStandardCount = 0;

    void incrementStandard(const std::string &attr) {
        standardAttributes[attr]++;
    }

    void incrementNonStandard() {
        nonStandardCount++;
    }

    void printResults() const {
        for (const auto &entry : standardAttributes) {
            llvm::outs() << entry.second << " " << entry.first << "\n";
        }
        llvm::outs() << nonStandardCount << " all non-standard attributes (combined)\n";
    }
};


class AttributeVisitor : public clang::RecursiveASTVisitor<AttributeVisitor> {
public:
    explicit AttributeVisitor(clang::ASTContext& Context) : Context(&Context) {}

    bool VisitCXXRecordDecl(clang::CXXRecordDecl *ClassDecl) {
        for (const auto *Attr : ClassDecl->attrs()) {
            processAttr(Attr);
        }
        return true;
    }

    bool VisitFunctionDecl(clang::FunctionDecl *FuncDecl) {
        for (const auto *Attr : FuncDecl->attrs()) {
            processAttr(Attr);
        }
        return true;
    }

    bool VisitVarDecl(clang::VarDecl *VarDecl) {
        for (const auto *Attr : VarDecl->attrs()) {
            processAttr(Attr);
        }
        return true;
    }

    void processAttr(const clang::Attr *attribute) {
        if (const auto *CXX11Attr = llvm::dyn_cast<clang::CXX11Attr>(attribute)) {
            std::string attrName = CXX11Attr->getAttrName()->getName().str();
            if (isStandardAttribute(attrName)) {
                counter.incrementStandard(attrName);
            } else {
                counter.incrementNonStandard();//存在这种可能吗？
            }
        } else {
            counter.incrementNonStandard();
        }
    }

    void printResults() const {
        counter.printResults();
    }

private:
    clang::ASTContext *Context;
    AttributeCounter counter;

    bool isStandardAttribute(const std::string &name) const {
        static const std::set<std::string> standardAttributes = {
            "noreturn", "deprecated", "fallthrough", "nodiscard",
            "maybe_unused", "likely", "unlikely", "carries_dependency",
            "no_unique_address", "using", "assume"
        };
        return standardAttributes.find(name) != standardAttributes.end();
    }
};//似乎这个思路有点子问题，clang已经内置了很多C++ standard attributes，这里应该直接使用clang的API来判断是否是standard attribute


class AttributeConsumer : public clang::ASTConsumer {
public:
    void HandleTranslationUnit(clang::ASTContext &Context) override {
		AttributeVisitor Visitor(Context);
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
        Visitor.printResults();
    }
};

class MyFrontendAction : public clang::ASTFrontendAction {
public:
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance& compInstance, clang::StringRef) final {
		return std::unique_ptr<clang::ASTConsumer>{new AttributeConsumer};
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
