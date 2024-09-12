#include <iostream>
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Frontend/FrontendAction.h"

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

class IfCounterCallback : public cam::MatchFinder::MatchCallback {
public:
    // 记录函数中的if语句数量
    std::map<std::string, int> FunctionIfCount;

    virtual void run(const cam::MatchFinder::MatchResult &Result) override {
        if (const clang::FunctionDecl *Func = Result.Nodes.getNodeAs<clang::FunctionDecl>("function")) {
            const auto &SM = *Result.SourceManager;
            if (!SM.isInMainFile(Func->getBeginLoc())) return;

            std::string FunctionName = Func->getQualifiedNameAsString();
            if (FunctionIfCount.find(FunctionName) == FunctionIfCount.end()) {
                FunctionIfCount[FunctionName] = 0;
            }
        }

        if (const clang::IfStmt *If = Result.Nodes.getNodeAs<clang::IfStmt>("ifStmt")) {
            const clang::FunctionDecl *Func = Result.Nodes.getNodeAs<clang::FunctionDecl>("function");
            if (Func) {
                std::string FunctionName = Func->getQualifiedNameAsString();
                FunctionIfCount[FunctionName]++;
            }
        }
    }

    // 输出每个函数的if语句统计
    void onEndOfTranslationUnit() override {
        for (const auto &entry : FunctionIfCount) {
            std::cout << entry.second << " " << entry.first << std::endl;
        }
    }
};

static llvm::cl::OptionCategory optionCategory("Tool options");


int main(int argc, const char **argv) {
	auto expectedParser = ct::CommonOptionsParser::create(argc, argv,
	  optionCategory);
	if (!expectedParser) {
		llvm::errs() << llvm::toString(expectedParser.takeError());
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = expectedParser.get();
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());

    IfCounterCallback IfCounter;
    cam::MatchFinder Finder;

    // 匹配函数声明并匹配其内部的 if 语句
    Finder.addMatcher(
        cam::ifStmt(cam::hasAncestor(cam::functionDecl(cam::isDefinition()).bind("function"))).bind("ifStmt"),
        &IfCounter
    );

    return tool.run(ct::newFrontendActionFactory(&Finder).get());
}
