#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/CommandLine.h"
#include <clang/Tooling/CommonOptionsParser.h>
#include <map>

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

// 定义存储函数的最大嵌套深度的结构
std::map<const clang::FunctionDecl*, int> FuncMaxDepthMap;

// 递归计算循环的嵌套深度
int calculateNestedDepth(const clang::Stmt* loopStmt, const cam::MatchFinder::MatchResult& Result) {
    const clang::Stmt* parent = Result.Context->getParents(*loopStmt)[0].get<clang::Stmt>();
    int depth = 0;

    // 向上查找是否有祖先是 forStmt, whileStmt 或 doStmt
    while (parent) {
        if (llvm::isa<clang::ForStmt>(parent) || llvm::isa<clang::WhileStmt>(parent) || llvm::isa<clang::DoStmt>(parent)) {
            ++depth;
        }

        // 获取下一个父节点
        auto parentList = Result.Context->getParents(*parent);
        if (parentList.empty()) break;
        parent = parentList[0].get<clang::Stmt>();
    }

    return depth;
}

class LoopMatchCallback : public cam::MatchFinder::MatchCallback {
public:
    virtual void run(const cam::MatchFinder::MatchResult &Result) override {
        const clang::SourceManager &SM = *Result.SourceManager;

        // 匹配到的 forStmt
        if (const auto *ForLoop = Result.Nodes.getNodeAs<clang::ForStmt>("forLoopInFunc")) {
            clang::SourceLocation Loc = ForLoop->getBeginLoc();
            llvm::outs() << "For loop found at line: " 
                         << SM.getSpellingLineNumber(Loc) << ":"
                         << SM.getSpellingColumnNumber(Loc) << "\n";

            // 计算嵌套深度
            int nestedDepth = calculateNestedDepth(ForLoop, Result);
            llvm::outs() << "For loop nested depth: " << nestedDepth << "\n";

            // 找到其所属的 FunctionDecl 并记录最大嵌套深度
            if (const auto *FuncDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("funcDecl")) {
                FuncMaxDepthMap[FuncDecl] = std::max(FuncMaxDepthMap[FuncDecl], nestedDepth + 1);
            }
        }

        // 匹配到的 whileStmt
        if (const auto *WhileLoop = Result.Nodes.getNodeAs<clang::WhileStmt>("whileLoopInFunc")) {
            clang::SourceLocation Loc = WhileLoop->getBeginLoc();
            llvm::outs() << "While loop found at line: " 
                         << SM.getSpellingLineNumber(Loc) << ":"
                         << SM.getSpellingColumnNumber(Loc) << "\n";

            // 计算嵌套深度
            int nestedDepth = calculateNestedDepth(WhileLoop, Result);
            llvm::outs() << "While loop nested depth: " << nestedDepth << "\n";

            // 找到其所属的 FunctionDecl 并记录最大嵌套深度
            if (const auto *FuncDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("funcDecl")) {
                FuncMaxDepthMap[FuncDecl] = std::max(FuncMaxDepthMap[FuncDecl], nestedDepth + 1);
            }
        }

        // 匹配到的 doStmt
        if (const auto *DoWhileLoop = Result.Nodes.getNodeAs<clang::DoStmt>("doWhileLoopInFunc")) {
            clang::SourceLocation Loc = DoWhileLoop->getBeginLoc();
            llvm::outs() << "Do-while loop found at line: " 
                         << SM.getSpellingLineNumber(Loc) << ":"
                         << SM.getSpellingColumnNumber(Loc) << "\n";

            // 计算嵌套深度
            int nestedDepth = calculateNestedDepth(DoWhileLoop, Result);
            llvm::outs() << "Do-while loop nested depth: " << nestedDepth << "\n";

            // 找到其所属的 FunctionDecl 并记录最大嵌套深度
            if (const auto *FuncDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("funcDecl")) {
                FuncMaxDepthMap[FuncDecl] = std::max(FuncMaxDepthMap[FuncDecl], nestedDepth + 1);
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
    ct::CommonOptionsParser& optionsParser = expectedParser.get();
    ct::ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

    cam::MatchFinder Finder;
    LoopMatchCallback Callback;

    // 匹配 for 循环，且其祖先是主文件中的任意函数声明 (FunctionDecl)
    Finder.addMatcher(
        cam::forStmt(
            cam::hasAncestor(cam::functionDecl().bind("funcDecl")),
            cam::isExpansionInMainFile()  // 只在主文件里匹配
        ).bind("forLoopInFunc"),
        &Callback
    );

    // 匹配 while 循环，且其祖先是主文件中的任意函数声明 (FunctionDecl)
    Finder.addMatcher(
        cam::whileStmt(
            cam::hasAncestor(cam::functionDecl().bind("funcDecl")),
            cam::isExpansionInMainFile()  // 只在主文件里匹配
        ).bind("whileLoopInFunc"),
        &Callback
    );

    // 匹配 do-while 循环，且其祖先是主文件中的任意函数声明 (FunctionDecl)
    Finder.addMatcher(
        cam::doStmt(
            cam::hasAncestor(cam::functionDecl().bind("funcDecl")),
            cam::isExpansionInMainFile()  // 只在主文件里匹配
        ).bind("doWhileLoopInFunc"),
        &Callback
    );

    int result = tool.run(clang::tooling::newFrontendActionFactory(&Finder).get());

    // 输出每个函数的最大嵌套深度
    for (const auto &entry : FuncMaxDepthMap) {
        llvm::outs() << "Function " << entry.first->getNameAsString()
                     << " has max nested loop depth: " << entry.second << "\n";
    }

    return result;
}

