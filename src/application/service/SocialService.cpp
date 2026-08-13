#include "application/service/SocialService.hpp"

namespace arcane::application::service {

vo::OperationResultVO SocialService::startPrivateChat(const std::string& memberId) const
{
    return memberId.empty()
        ? vo::OperationResultVO{false, "Select a campus member before opening a private chat."}
        : vo::OperationResultVO{true, "Private chat requested for " + memberId + "."};
}

vo::OperationResultVO SocialService::viewProfile(const std::string& memberId) const
{
    return memberId.empty()
        ? vo::OperationResultVO{false, "Select a campus member before viewing a profile."}
        : vo::OperationResultVO{true, "Profile requested for " + memberId + "."};
}

} // namespace arcane::application::service
