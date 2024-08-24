#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

namespace ct = clang::tooling;

class FindPragmas : public clang::PPCallbacks {
public:
    explicit FindPragmas(clang::SourceManager& sourceManager, int& pragmaCount) 
        : sourceManager_(&sourceManager), pragmaCount_(pragmaCount) {}

    void PragmaDirective(clang::SourceLocation loc,
                         clang::PragmaIntroducerKind introducer) override {
        if (!sourceManager_->isInMainFile(loc)) return;
        pragmaCount_++;
    }

private:
    clang::SourceManager* sourceManager_;
    int& pragmaCount_;
};

class PragmaFinderAction : public clang::PreprocessOnlyAction {
public:
    explicit PragmaFinderAction(int& pragmaCount) : pragmaCount_(pragmaCount) {}

protected:
    bool BeginSourceFileAction(clang::CompilerInstance& ci) override {
        std::unique_ptr<FindPragmas> findPragmas(
            new FindPragmas(ci.getSourceManager(), pragmaCount_));
        clang::Preprocessor& pp = ci.getPreprocessor();
        pp.addPPCallbacks(std::move(findPragmas));
        return true;
    }

private:
    int& pragmaCount_;
};

#pragma message("This is a pragma message")
#pragma message("This is a pragma message")
#pragma message("This is a pragma message")
#pragma message("This is a pragma message")


// 自定义的 FrontendActionFactory
class PragmaFinderActionFactory : public ct::FrontendActionFactory {
public:
    explicit PragmaFinderActionFactory(int& pragmaCount) : pragmaCount_(pragmaCount) {}

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<PragmaFinderAction>(pragmaCount_);
    }

private:
    int& pragmaCount_;
};

int main(int argc, char** argv) {
    if (argc < 2) {
        llvm::errs() << "Usage: " << argv[0] << " <compile_commands.json>\n";
        return 1;
    }

    std::string pathname = argv[1];
    std::string errString;
    std::unique_ptr<ct::CompilationDatabase> compDatabase =
        ct::JSONCompilationDatabase::loadFromFile(pathname, errString,
                                                  ct::JSONCommandLineSyntax::AutoDetect);

    if (!compDatabase) {
        llvm::errs() << std::format("ERROR: {}\n", errString);
        return 1;
    }

    // 获取编译数据库中的所有源文件
    std::vector<std::string> sourcePaths = compDatabase->getAllFiles();
    llvm::outs() << std::format("Number of files: {}\n", sourcePaths.size());

    std::string maxPragmaFile;
    int maxPragmaCount = 0;

    for (const auto& sourcePath : sourcePaths) {
        llvm::outs() << std::format("Processing file: {}\n", sourcePath);

        // 对每个源文件，计算 #pragma 指令的数量
        int pragmaCount = 0;
        ct::ClangTool tool(*compDatabase, {sourcePath});
        PragmaFinderActionFactory actionFactory(pragmaCount);
        int result = tool.run(&actionFactory);

        if (result != 0) {
            llvm::errs() << std::format("Error processing file: {}\n", sourcePath);
            continue;
        }

        if (pragmaCount > maxPragmaCount) {
            maxPragmaCount = pragmaCount;
            maxPragmaFile = sourcePath;
        }
    }

    // 输出包含最多 #pragma 指令的文件
    if (!maxPragmaFile.empty()) {
        llvm::outs() << std::format("\nFile with most #pragma directives: {}\nTotal #pragmas: {}\n",
                                    maxPragmaFile, maxPragmaCount);
    } else {
        llvm::outs() << "No #pragma directives found in any file.\n";
    }

    return 0;
}
