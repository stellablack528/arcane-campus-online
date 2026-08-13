#pragma once

#include "application/vo/CampusVO.hpp"

#include <string>

namespace arcane::application::service {

class SocialService final {
public:
    [[nodiscard]] vo::OperationResultVO startPrivateChat(const std::string& memberId) const;
    [[nodiscard]] vo::OperationResultVO viewProfile(const std::string& memberId) const;
};

} // namespace arcane::application::service
