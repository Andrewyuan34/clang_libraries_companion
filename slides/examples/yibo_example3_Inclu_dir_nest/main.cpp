#include <format>
#include <stack>
#include <vector>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

namespace ct = clang::tooling;
using namespace std::string_literals;

class FindIncludes : public clang::PPCallbacks {//不是，这个感觉是个特别大的逻辑啊。。好诡异。。。搞不清楚
public:
    FindIncludes(clang::SourceManager &sourceManager) : sourceManager_(&sourceManager) {}

    void InclusionDirective(clang::SourceLocation hashLoc,
                            const clang::Token &, llvm::StringRef fileName,
                            bool isAngled, clang::CharSourceRange,
                            clang::OptionalFileEntryRef file,
                            llvm::StringRef, llvm::StringRef,
                            const clang::Module *,
                            clang::SrcMgr::CharacteristicKind) override {

        std::string headerName = isAngled ? ("<"s + std::string(fileName) + ">"s)
                                          : ("\""s + std::string(fileName) + "\""s);

        // 压入栈中，表示嵌套层次
        includeStack_.push_back(headerName);

        // 检查是否为主文件中的 `#include` 指令
        if (sourceManager_->getFileID(hashLoc) == sourceManager_->getMainFileID()) {
            printIncludeStack();  // 打印当前包含栈
            llvm::outs() << std::format("    location\n");
            //includeStack_.pop_back();  // 弹出当前的文件名，回到上一级ssssssssssssssssssssssssssssss
        }
    }

private:
    // 打印包含栈
    void printIncludeStack() const {
        std::string includeHierarchy;
        for (auto i = includeStack_.begin(); i != includeStack_.end(); ++i) {
            // 格式化输出
            includeHierarchy += (i != includeStack_.begin() ? " -> " : "") + *i;
        }
        // 打印包含栈信息
        llvm::outs() << includeHierarchy << '\n';
    }

    clang::SourceManager *sourceManager_;
    std::vector<std::string> includeStack_;
};

class IncludeFinderAction : public clang::PreprocessOnlyAction {
    bool BeginSourceFileAction(clang::CompilerInstance &ci) override {
        std::unique_ptr<FindIncludes> findIncludes(new FindIncludes(ci.getSourceManager()));
        clang::Preprocessor &pp = ci.getPreprocessor();
        pp.SetSuppressIncludeNotFoundError(true);
        pp.addPPCallbacks(std::move(findIncludes));
        return true;
    }
};

static llvm::cl::OptionCategory toolOptions("Tool Options");

int main(int argc, char **argv) {
    auto expectedOptionsParser = ct::CommonOptionsParser::create(argc,
                                                                 const_cast<const char **>(argv),
                                                                 toolOptions);
    if (!expectedOptionsParser) {
        llvm::errs() << std::format("Unable to create option parser ({}).\n",
                                    llvm::toString(expectedOptionsParser.takeError()));
        return 1;
    }
    ct::CommonOptionsParser &optionsParser = *expectedOptionsParser;
    ct::ClangTool tool(optionsParser.getCompilations(),
                       optionsParser.getSourcePathList());
    int status = tool.run(
        ct::newFrontendActionFactory<IncludeFinderAction>().get());
    if (status) {
        llvm::errs() << "error detected\n";
    }
    return !status ? 0 : 1;
}

