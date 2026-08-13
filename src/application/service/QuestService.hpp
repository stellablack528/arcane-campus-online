#pragma once

#include "application/do/CampusDO.hpp"
#include "application/dto/CampusDTO.hpp"
#include "application/vo/CampusVO.hpp"

#include <memory>

namespace arcane::database {
class QuestDAO;
}

namespace arcane::application::service {

class QuestService final {
public:
    explicit QuestService(std::shared_ptr<database::QuestDAO> questDao) noexcept;

    [[nodiscard]] vo::QuestListVO getCharacterQuests(std::uint64_t characterId) const;
    [[nodiscard]] vo::OperationResultVO updateProgress(const dto::QuestProgressRequestDTO& request) const;

private:
    std::shared_ptr<database::QuestDAO> questDao_;
};

} // namespace arcane::application::service
