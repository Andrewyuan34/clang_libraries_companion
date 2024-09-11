#include <format>
#include <string>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <clang/Analysis/CFG.h>
#include <clang/Analysis/AnalysisDeclContext.h>
#include <clang/Analysis/Analyses/LiveVariables.h>

namespace cam = clang::ast_matchers;
namespace ct = clang::tooling;
namespace lc = llvm::cl;

static lc::OptionCategory toolCategory("Tool Options");

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {//这里是不是得用多个matcher来匹配不同的节点？
    virtual void run(const cam::MatchFinder::MatchResult& result) final {
        if (const auto *funcDecl = result.Nodes.getNodeAs<clang::FunctionDecl>("func")) {
            clang::ASTContext *astContext = result.Context;
            clang::Stmt *funcBody = funcDecl->getBody();
            if (!funcBody) return;
        
            llvm::outs() << std::format("FUNCTION: {}\n", funcDecl->getQualifiedNameAsString());

            // 获取当前函数的 CFG
            clang::AnalysisDeclContextManager manager(*astContext);
            clang::AnalysisDeclContext *AC = manager.getContext(funcDecl);
            AC->getCFGBuildOptions().setAllAlwaysAdd();//why？？？？加了这行以后就可以生成正常CFG了
            //AC->getCFGBuildOptions().AddLifetime = true;
            const clang::CFG *cfg = AC->getCFG();

            if (!cfg) {
                llvm::errs() << "Could not generate CFG for function.\n";
                return;
            }

            // 构建 LiveVariables 分析器
            clang::LiveVariables *liveVars = AC->getAnalysis<clang::LiveVariables>(); 
	        if (!liveVars) return;
            auto observer = std::make_unique<clang::LiveVariables::Observer>();
	        assert(observer);
	        liveVars->runOnAllBlocks(*observer);
            /*auto livenessvalues = std::make_unique<clang::LiveVariables::LivenessValues>();
            assert(livenessvalues);*/
	        //liveVars->dumpBlockLiveness((funcDecl->getASTContext()).getSourceManager());

            // 遍历 CFG 中的每个块，检查是否有死存储
            for (const clang::CFGBlock *block : *cfg) {
                for (auto &elem : *block) {
                    // 获取 CFGStmt
                    if (std::optional<clang::CFGStmt> cfgStmtOpt = elem.getAs<clang::CFGStmt>()) {
                        const clang::CFGStmt &cfgStmt = *cfgStmtOpt;
                        const clang::Stmt *stmt = cfgStmt.getStmt();
                            // 检查赋值操作
                            if (const clang::BinaryOperator *binOp = llvm::dyn_cast<clang::BinaryOperator>(stmt)) {
                                // 只关注赋值操作
                                if (binOp->isAssignmentOp()) {
                                    // 获取 LHS（左值表达式）
                                    if (const clang::DeclRefExpr *lhs = llvm::dyn_cast<clang::DeclRefExpr>(binOp->getLHS())) {
                                        // 获取 VarDecl
                                        if (const clang::VarDecl *varDecl = llvm::dyn_cast<clang::VarDecl>(lhs->getDecl())) {
                                            // 检查变量在当前块的末尾是否活跃
                                            if (!liveVars->isLive(block, varDecl)) {
                                                llvm::outs() << "Dead store detected in function '"
                                                             << funcDecl->getQualifiedNameAsString() << "' at "
                                                             << varDecl->getLocation().printToString(*result.SourceManager)//这里的输出是定义的位置，而不是使用的位置
                                                             << "\n";
                                                llvm::outs() << "Variable name: " << varDecl->getNameAsString() << "\n";
                                                if (varDecl->getType().getAsString() != "") {
                                                    llvm::outs() << "Variable type: " << varDecl->getType().getAsString() << "\n";
                                                }
                                                llvm::outs() << "Assigned at: ";
                                                binOp->getSourceRange().print(llvm::outs(), *result.SourceManager);
                                                llvm::outs() << "\n";
                                                llvm::outs() << "Statement: '";
                                                binOp->printPretty(llvm::outs(), nullptr, clang::PrintingPolicy(astContext->getLangOpts()));
                                                //varDecl->getSourceRange().print(llvm::outs(), *result.SourceManager);
                                                //lhs->printPretty(llvm::outs(), nullptr, clang::PrintingPolicy(astContext->getLangOpts()));
                                            }
                                        }
                                    }
                            }
                        }
                    }
                }
            }
        }
    }
};

// 匹配器函数：匹配所有函数定义
cam::DeclarationMatcher getFuncMatcher() {
    return cam::functionDecl().bind("func");
}

int main(int argc, const char **argv) {
    llvm::Expected<ct::CommonOptionsParser> expOptionsParser = ct::CommonOptionsParser::create(argc, argv, toolCategory);
    if (!expOptionsParser) {
        llvm::errs() << llvm::toString(expOptionsParser.takeError());
        return 1;
    }

    ct::CommonOptionsParser& optionsParser = *expOptionsParser;
    ct::ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

    cam::DeclarationMatcher funcMatcher = getFuncMatcher();
    MyMatchCallback matchCallback;
    cam::MatchFinder finder;

    finder.addMatcher(funcMatcher, &matchCallback);

    int status = tool.run(ct::newFrontendActionFactory(&finder).get());
    if (status) {
        llvm::errs() << "error occurred\n";
    }

    return !status ? 0 : 1;
}

/*
所以这道题的整体思路就是：

Liveness analysis 通过分析控制流，确定每个变量在每个程序点上的活跃性信息。
如果一个变量在某个程序点上是非活跃的，说明它的值在接下来的代码中不会被使用。
Dead store detection 使用 liveness 信息来查找死存储：
如果某个变量在赋值操作之后是非活跃的，且在下一次赋值之前没有被读取或使用，那么这次赋值就是死存储。
换句话说，编译器可以通过 liveness analysis 识别出赋值语句是否是无效的（没有被读取）。
*/

/*
然后现在的情况是
1.  可以顺利的通过AST找到在这种格式下的node
    BinaryOperator 
        | |-DeclRefExpr 
    但是并不能找到
    DeclStmt 
        | `-VarDecl
    这种格式下的node，所以该怎么解决？
    但本质上实际找的是vardecl？
这个暂时还没想到解决办法。目前的方法只可以处理单纯由BinaryOperator构成的赋值语句。因此return 语句造成错误的返回值。

2. 虽然能找到第一种node，但使用if (!liveVars->isLive(stmt, varDecl)) 并不能如我所想的那样得到基于stmt的vardecl的liveness信息。
[Solved] 可以直接用block就行，因为生成的cfg本身就是在已找到的func的基础上生成的，因此可以直接迭代每个block就可以获得正确的想要的islive信息。

3. Return的表达式并不应该被判定为deadstore，但是现在的代码会判定为deadstore。
[partly solved] 发现CFG生成的block里的element是按照倒着AST的顺序排列的，因此可以通过判断是否是block的最后一个element来判断是否是returnStmt。但目前还没有实现
*/

/*  
这里还挺重要的，通过BinaryOperator得到DeclRefExpr，然后通过DeclRefExpr得到VarDecl，然后通过VarDecl得到变量的名字和类型。
这条通路，看似直接，但实际从DeclRefExpr得到VarDecl的过程中，返回的VarDecl是对应该Variable的定义/声明的位置，而不是使用的位置。
（引申出来，如果有多个定义呢？是回到最初的点，还是回到上一次赋值的点？）Ok 已确认是回到最初的定义的位置。
*/