#include "commands/dispatcher.hpp"

#include <fstream>
#include <random>
#include <filesystem>

#include "editor/editor.hpp"

namespace tui_demo {
namespace {
    //temp solution until file name becomes a thing
    std::string getPathStamp() {
        std::random_device seed;
        std::mt19937 gen(seed());
        std::uniform_int_distribution<int> distrib(1, 100);

        std::string randomNum = std::to_string(distrib(gen));
        return randomNum;
    }
}

    CommandResult CommandDispatcher::Execute(const NewCommand& command) {
        const auto& currentDocument = editor_.State().currentDoc;
        const std::string newFileName = "newFile";
        const std::filesystem::path newPath = currentDocument.file.parent_path() 
            / (newFileName + getPathStamp()); 

        std::ofstream file(
            newPath, 
            std::ios::out |  
            std::ios::binary
        );

        if (!file) {
            return {CommandError::WriteFailed, "Cannot create a new file"};
        }

        file << currentDocument.text;

        if (!file) {
            return {CommandError::WriteFailed, "Cannot write to new file"};
        }

        return{};
    }
}