#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

class TemplateMatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override {
        const clang::SourceManager &SM = *Result.SourceManager;

        if (const auto *ClassTemplate = Result.Nodes.getNodeAs<clang::ClassTemplateSpecializationDecl>("classTemplateInstantiation")) {
            clang::SourceLocation InstLoc = ClassTemplate->getPointOfInstantiation();
            
            if (InstLoc.isValid()) {  // 检查是否是有效的位置
                llvm::outs() << "Class template instantiation: " << ClassTemplate->getNameAsString()
                             << " instantiated at " << SM.getSpellingLineNumber(InstLoc) << ":"
                             << SM.getSpellingColumnNumber(InstLoc) << "\n";
            } else {
                llvm::outs() << "Class template instantiation: " << ClassTemplate->getNameAsString()
                             << " but no instantiation location found.\n";
            }
        } else if (const auto *FunctionTemplate = Result.Nodes.getNodeAs<clang::FunctionDecl>("functionInstantiation")) {
            clang::SourceLocation InstLoc = FunctionTemplate->getPointOfInstantiation();

            if(InstLoc.isValid()) {
                llvm::outs() << "Function template instantiation: " << FunctionTemplate->getNameAsString()
                             << " instantiated at " << SM.getSpellingLineNumber(InstLoc) << ":"
                             << SM.getSpellingColumnNumber(InstLoc) << "\n";
            } else {
                llvm::outs() << "Function template instantiation: " << FunctionTemplate->getNameAsString()
                             << " but no instantiation location found.\n";
            }
        }
    }
};



static llvm::cl::OptionCategory optionCategory("Tool options");

int main(int argc, const char **argv) {
    auto expectedParser = ct::CommonOptionsParser::create(argc, argv, optionCategory);
    if (!expectedParser) {
        llvm::errs() << llvm::toString(expectedParser.takeError());
        return 1;
    }
    ct::CommonOptionsParser &optionsParser = expectedParser.get();
    ct::ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

    cam::MatchFinder Finder;
    TemplateMatchCallback Callback;

    // Matcher for class template instantiations
    Finder.addMatcher(cam::classTemplateSpecializationDecl(cam::isInstantiated(), cam::isExpansionInMainFile()).bind("classTemplateInstantiation"), &Callback);
    Finder.addMatcher(cam::functionDecl(cam::isInstantiated(), cam::isExpansionInMainFile()).bind("functionInstantiation"), &Callback);
    // 执行匹配
    tool.run(ct::newFrontendActionFactory(&Finder).get());

    return 0;
}
