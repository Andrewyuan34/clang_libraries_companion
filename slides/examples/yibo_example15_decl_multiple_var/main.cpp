/************************************************
 * 15. The following may or may not be possible:
 * find each occurrence of a declaration that declares more than one variable
 * for global and data members cannot tell from AST (I think)
 * might need to use source location?
 * 不是很确定，但思路就是对于每一个decl，找到同一行且同一列开始的decl，存下开始和结束的col，输出。
 * 似乎不需要特殊处理在func里的decl（会被放在declstmt下）
 * 好像更多是数据结构的问题
 * 现在matcher已经写好了。
 * callback里需要单独处理vardecl和fielddecl
 * 进入run以后，获得vardecl或fielddecl，然后获取source location，存到栈里，然后再找下一个vardecl或fielddecl，
 * 如果在同一行并且开始的列一样，那么就更新栈顶的end col，如果不在同一行，那么就输出栈顶的信息，然后清空栈。
 * 为什么使用clang::CharSourceRange::getTokenRange可以呢？
 * 
 */

#include <format>
#include <stack>
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Lex/Lexer.h"

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

// 定义命令行选项类别
static llvm::cl::OptionCategory optionCategory("Tool options");

// 结构体用于保存变量声明的信息
struct DeclInfo {
    unsigned Line;
    unsigned StartCol;
    unsigned EndCol;
    llvm::StringRef SourceText;
};

// 回调类，用于处理匹配到的 VarDecl 和 FieldDecl 节点
class VarDeclFieldDeclCallback : public cam::MatchFinder::MatchCallback {
public:
    virtual void run(const cam::MatchFinder::MatchResult &Result) {
        // 获取 SourceManager
        const clang::SourceManager &SM = Result.Context->getSourceManager();

        // 处理 VarDecl 节点
        if (const clang::VarDecl *varDecl = Result.Nodes.getNodeAs<clang::VarDecl>("varDecl")) {
            handleDeclaration(varDecl, SM);
        }

        // 处理 FieldDecl 节点
        if (const clang::FieldDecl *fieldDecl = Result.Nodes.getNodeAs<clang::FieldDecl>("fieldDecl")) {
            handleDeclaration(fieldDecl, SM);
        }
    }

    int onEndofCallback(){
        if(outputflag){
            llvm::outs() << "Find a decl with multiple variables in one line: \n";
            llvm::outs() << "Line: " << declStack.top().Line
                         << ", Start Column: " << declStack.top().StartCol
                         << ", End Column: " << declStack.top().EndCol << "\n";
            llvm::outs() << declStack.top().SourceText << "\n\n";
            outputflag = false;
        }
        return 0;
    }

private:
    // 栈用于保存当前行的声明信息
    std::stack<DeclInfo> declStack;
    bool outputflag = false;

    template<typename DeclType>
    void handleDeclaration(const DeclType *decl, const clang::SourceManager &SM) {
        clang::SourceRange range = decl->getSourceRange();
        unsigned Line = SM.getSpellingLineNumber(range.getBegin());
        unsigned startCol = SM.getSpellingColumnNumber(range.getBegin());
        unsigned endCol = SM.getSpellingColumnNumber(range.getEnd());

        // If the stack is empty, first push the current declaration info into the stack
        // If the current declaration is in the same line and has the same start column as the top of the stack
        // update the end column of the top of the stack
        if(declStack.top().Line == Line && declStack.top().StartCol == startCol){
            declStack.top().EndCol = endCol;
            outputflag = true;
            llvm::StringRef SourceText = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(range), SM, clang::LangOptions());
            declStack.top().SourceText = SourceText;
        }

        if(declStack.top().Line != Line || declStack.top().StartCol != startCol){
            if(outputflag){
                llvm::outs() << "Find a decl with multiple variables in one line: \n";
                llvm::outs() << "Line: " << declStack.top().Line
                             << ", Start Column: " << declStack.top().StartCol
                             << ", End Column: " << declStack.top().EndCol << "\n";
                llvm::outs() << declStack.top().SourceText << "\n\n";
                declStack.push({Line, startCol, endCol, {}});
                outputflag = false;
            }else{
                declStack.push({Line, startCol, endCol, {}});
            }
        }

        // Can't put this in the if statement above 
        if(declStack.empty()){
            declStack.push({Line, startCol, endCol, {}});
        }


    }
};

int main(int argc, const char **argv) {
    auto expectedParser = ct::CommonOptionsParser::create(argc, argv, optionCategory);
    if (!expectedParser) {
        llvm::errs() << llvm::toString(expectedParser.takeError());
        return 1;
    }
    ct::CommonOptionsParser& optionsParser = expectedParser.get();
    ct::ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

    cam::MatchFinder Finder;
    VarDeclFieldDeclCallback Callback;

    // 创建一个匹配器，匹配 VarDecl 或 FieldDecl
    auto varDeclMatcher = cam::varDecl(cam::isExpansionInMainFile()).bind("varDecl");
    auto fieldDeclMatcher = cam::fieldDecl(cam::isExpansionInMainFile()).bind("fieldDecl");
    Finder.addMatcher(varDeclMatcher, &Callback);
    Finder.addMatcher(fieldDeclMatcher, &Callback);


    int result = tool.run(clang::tooling::newFrontendActionFactory(&Finder).get());
    assert(!result);
    Callback.onEndofCallback();

    return result;
}

