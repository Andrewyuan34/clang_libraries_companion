#include <format>
#include <string>
#include <clang/Analysis/CFG.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include "utilities.hpp"

namespace cam = clang::ast_matchers;
namespace ct = clang::tooling;
namespace lc = llvm::cl;

static lc::OptionCategory toolCategory("Tool Options");

class UnreachableStmtAnalyzer : public cam::MatchFinder::MatchCallback {
public:
    // 在匹配到的函数声明时执行
    virtual void run(const cam::MatchFinder::MatchResult &Result) final {
		const auto& sm = *Result.SourceManager;
        if (const clang::FunctionDecl *FD = Result.Nodes.getNodeAs<clang::FunctionDecl>("func")) {
			if(!sm.isWrittenInMainFile(FD->getLocation())) {return;}

            if (FD->hasBody()) {
                clang::Stmt *Body = FD->getBody();
				if(!Body) {
					llvm::outs() << "Function body is empty\n";
					return;}

                // 生成控制流图（CFG）
                std::unique_ptr<clang::CFG> cfg = clang::CFG::buildCFG(FD, Body, 
                                                                       Result.Context, clang::CFG::BuildOptions());

                if (!cfg) {
                    llvm::errs() << "Failed to generate CFG for function: " 
                                 << FD->getNameInfo().getAsString() << "\n";
                    return;
                }

                // The reason why dealing with the unreachable statements in this way is that if we match stmt in CFGElem, 
                // we might get the same line of code multiple times. So we need to filter out the duplicate lines of code.

                const clang::Stmt* lastStmt = nullptr;
                clang::SourceLocation lastLoc;
                clang::SourceRange lastRange;
                unsigned lastLine = 0;
                // 定义一个 lambda，用于格式化输出
                auto printUnreachableStatement = [&](const clang::SourceLocation& loc, const clang::SourceRange& range) {
                    llvm::outs() << std::format(
                        "---------------------------------\n"
                        "Unreachable statement found at {}\n"
                        "Source range: {}\n"
                        "{}\n"
                        "---------------------------------\n",
                        locationToString(sm, loc),
                        rangeToString(sm, range),
                        addLineNumbers(getSourceText(sm, range), sm.getSpellingLineNumber(loc))
                    );
                };

                // 遍历 CFG 中的所有基本块
                for (const clang::CFGBlock *Block : *cfg) {
                    bool isReachable = !Block->pred_empty(); // 判断该块是否有前驱节点

                    // 只处理不可达块
                    if (!isReachable) {
                        for (const auto &Element : *Block) {
                            if (auto cfgStmt = Element.getAs<clang::CFGStmt>()) {
                                const clang::Stmt *stmt = cfgStmt->getStmt();
                                clang::SourceLocation loc = stmt->getBeginLoc();

                                // 获取当前语句的行号
                                unsigned currentLine = sm.getSpellingLineNumber(loc);

                                // 如果这是不同行的语句，输出前一行的最后一个语句
                                if (lastStmt && currentLine != lastLine) {
                                    printUnreachableStatement(lastLoc, lastRange);  // 调用 lambda 输出不可达语句
                                }

                                // 更新最后一个语句的位置信息
                                lastStmt = stmt;
                                lastLoc = loc;
                                lastRange = stmt->getSourceRange();
                                lastLine = currentLine;
                            }
                        }
                    }
                }

                // After the loop, if the last statement is unreachable, output it
                if (lastStmt) {
                    printUnreachableStatement(lastLoc, lastRange);  // 调用 lambda 输出不可达语句
                }
            }
        }
    }
};

int main(int argc, const char **argv) {
    // 创建并解析命令行参数
    llvm::Expected<ct::CommonOptionsParser> expOptionsParser =
    ct::CommonOptionsParser::create(argc, argv, toolCategory);
    if (!expOptionsParser) {
        llvm::errs() << llvm::toString(expOptionsParser.takeError());
        return 1;
    }
    ct::CommonOptionsParser& optionsParser = *expOptionsParser;
    ct::ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

    // 创建匹配器和回调
    UnreachableStmtAnalyzer Analyzer;
    cam::MatchFinder finder;
    finder.addMatcher(cam::functionDecl(cam::hasBody(cam::stmt())).bind("func"), &Analyzer);//这里不熟悉需要查文档，似乎有特别特别多matcher可以用，这里应该需要improve

    // 运行工具
    int status = tool.run(ct::newFrontendActionFactory(&finder).get());
    if (status) {
        llvm::errs() << "Error occurred\n";
    }
    return !status ? 0 : 1;
}
