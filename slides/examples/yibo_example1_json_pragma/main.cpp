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

// 自定义的 FrontendActionFactory工厂类，用于创建自定义的需要参数的 FrontendAction 对象
class PragmaFinderActionFactory : public ct::FrontendActionFactory {
public:
    explicit PragmaFinderActionFactory(int& pragmaCount) : pragmaCount_(pragmaCount) {}

    std::unique_ptr<clang::FrontendAction> create() override { //注意这里的返回值，也是一个多态的应用
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

/*
多态性分析
FrontendAction 是基类：

clang::FrontendAction 是一个抽象基类，用于定义 Clang 工具链中各种前端操作（如编译、预处理、语法树生成等）的接口。这个基类提供了一些纯虚函数，需要在派生类中实现。
PragmaFinderAction 是派生类：

PragmaFinderAction 类继承自 clang::PreprocessOnlyAction，该类是 clang::FrontendAction 的派生类，专门用于处理预处理器相关的操作。在 PragmaFinderAction 中，你实现了与特定任务相关的逻辑，例如统计 #pragma 指令。
FrontendActionFactory 是用于创建 FrontendAction 对象的工厂：

ct::FrontendActionFactory 是一个抽象工厂类，提供了创建 FrontendAction 对象的接口。工厂模式允许你根据需要灵活地创建不同的 FrontendAction 派生类对象。
PragmaFinderActionFactory 是一个具体工厂类：

PragmaFinderActionFactory 继承自 ct::FrontendActionFactory，并重写了 create() 方法，用于创建具体的 PragmaFinderAction 对象。
多态的应用：

在 tool.run(&actionFactory) 中，tool.run 函数接收一个指向 FrontendActionFactory 基类的指针 &actionFactory，并调用该工厂的 create() 方法来生成 FrontendAction 对象。这就是多态性发挥作用的地方，因为实际调用的是 PragmaFinderActionFactory::create() 方法，而该方法返回的是一个 PragmaFinderAction 对象。
由于 tool.run() 函数在编译时并不知道 actionFactory 的具体类型（它只知道 actionFactory 是一个 FrontendActionFactory），多态性允许它在运行时调用实际的派生类方法（即 PragmaFinderActionFactory::create()）。
总结
多态性：通过使用基类指针或引用来调用派生类的重写函数实现多态性，这使得代码更加灵活和可扩展。
工厂模式：通过工厂模式创建具体的 FrontendAction 对象，有助于将对象创建过程与使用过程解耦，这进一步增强了代码的可维护性和可扩展性。
*/