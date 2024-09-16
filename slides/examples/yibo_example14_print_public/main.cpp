#include <format>
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/CommandLine.h"
#include <clang/Tooling/CommonOptionsParser.h>

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

std::string getSourceTextByLineRange(const clang::SourceManager& SM, unsigned startLine, unsigned endLine) {
    std::string result;

    // Get the buffer data from the SourceManager
    llvm::StringRef bufferData = SM.getBufferData(SM.getMainFileID());
    std::istringstream stream(bufferData.str());
    std::string line;
    unsigned currentLine = 1;

    while (std::getline(stream, line)) {
        if (currentLine >= startLine && currentLine <= endLine) {
            result += std::format("{:4}: {}\n", currentLine, line);
        }
        currentLine++;
        if (currentLine > endLine) break;  // Stop reading if past the end line
    }

    return result;
}

void processAccessSpec(const bool isClass, const clang::AccessSpecDecl* AccessSpec, const clang::CXXRecordDecl *Record, const clang::SourceManager& SM) {
    static unsigned startLine = 0;
    static bool startcounting = false;
    static bool firstAccessSpec = true; // Only works for struct

    clang::SourceRange sourceRange = AccessSpec->getSourceRange();

    if(isClass){
        if (AccessSpec->getAccess() == clang::AS_public) {
            startLine = SM.getSpellingLineNumber(sourceRange.getBegin());
            startcounting = true;
        } else if (startcounting) { 
            unsigned endLine = SM.getSpellingLineNumber(sourceRange.getBegin());//这里还缺一些判断信息，比如如果是最后一个AccessSpecDecl，那么就要输出到最后一行
            llvm::outs() << getSourceTextByLineRange(SM, startLine, endLine - 1);
            startcounting = false;
        }
    }else{
        if (firstAccessSpec) {
            startLine = SM.getSpellingLineNumber(Record->getSourceRange().getBegin()) + 1;
            unsigned endLine = SM.getSpellingLineNumber(sourceRange.getBegin()) - 1;
            llvm::outs() << getSourceTextByLineRange(SM, startLine, endLine);
            firstAccessSpec = false;
        }else{
            if (AccessSpec->getAccess() == clang::AS_public) {
                startLine = SM.getSpellingLineNumber(sourceRange.getBegin());
                startcounting = true;
            } else if (startcounting) { 
                unsigned endLine = SM.getSpellingLineNumber(sourceRange.getBegin());//这里还缺一些判断信息，比如如果是最后一个AccessSpecDecl，那么就要输出到最后一行
                llvm::outs() << getSourceTextByLineRange(SM, startLine, endLine - 1);
                startcounting = false;
            }
        }
    }
//问题主要在这个函数里
}


class CXXRecordCallback : public cam::MatchFinder::MatchCallback {
public:
    virtual void run(const cam::MatchFinder::MatchResult &Result) override {
        const clang::CXXRecordDecl *Record = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("classOrStruct");

        if (Record && !Record->isUnion()) {
            bool isClass = Record->isClass();
            bool isStruct = Record->isStruct();

            llvm::outs() << (isClass ? "Class: " : "Struct: ") << Record->getNameAsString() << "\n";

            const clang::SourceManager &sm = *Result.SourceManager;

            // Iterate through the declarations
            for (auto *Decl : Record->decls()) {
                if (const auto* AccessSpec = dyn_cast<clang::AccessSpecDecl>(Decl)) {
                    processAccessSpec(isClass, AccessSpec, Record, sm);
                } 
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
    CXXRecordCallback Callback;

    // Match CXXRecordDecl nodes (classes and structs)
    Finder.addMatcher(cam::cxxRecordDecl().bind("classOrStruct"), &Callback);

    int result = tool.run(clang::tooling::newFrontendActionFactory(&Finder).get());

    return result;
}

/*************************************************
14. specify one or more source files as command-line arguments
for each struct/class definition in source file:
print as C++ source code only the public interface of the class

对于这道题大致的思路如下：
用matcher匹配到class或struct。或者直接就用cxxrecorddecl匹配。进到run以后再区分是class还是struct。
对于class：
因为本身是除非明确标注public/potected，否则默认是private的
所以首先需要先找到public的AccessSpecDecl，之后再找到下一个AccessSpecDecl，这两者中间的行就是要输出public的内容。

对于struct：
因为本身是除非明确标注public/potected，否则默认是public的
所以最开始到第一个AccessSpecDecl之间的内容就是public的内容。然后其余和class一样。（所以这部分可以写成函数）

又想了一下，应该是可以把两个recorddecl的逻辑合并到一个函数里的

还缺一些对于细节的处理：
1. 如果是最后一个AccessSpecDecl，那么就要输出到最后一行
*/