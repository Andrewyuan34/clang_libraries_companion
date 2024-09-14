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
        if (const auto *ClassTemplate = Result.Nodes.getNodeAs<clang::ClassTemplateSpecializationDecl>("classTemplateInstantiation")) {
            ClassTemplate->dump(); // Just a simple dump for now
        }else llvm::outs() << "Error...\n";

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
    // 执行匹配
    tool.run(ct::newFrontendActionFactory(&Finder).get());

    return 0;
}
