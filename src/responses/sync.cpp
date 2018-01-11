#include "mtx/events/collections.hpp"
#include "mtx/responses/common.hpp"
#include "mtx/responses/sync.hpp"

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, State &state)
{
        utils::parse_state_events(obj.at("events"), state.events);
}

void
from_json(const json &obj, Timeline &timeline)
{
        timeline.prev_batch = obj.at("prev_batch").get<std::string>();
        timeline.limited    = obj.at("limited").get<bool>();

        utils::parse_timeline_events(obj.at("events"), timeline.events);
}

void
from_json(const json &obj, UnreadNotifications &notifications)
{
        if (obj.find("highlight_count") != obj.end())
                notifications.highlight_count = obj.at("highlight_count").get<uint16_t>();

        if (obj.find("notification_count") != obj.end())
                notifications.notification_count = obj.at("notification_count").get<uint16_t>();
}

void
from_json(const json &obj, Ephemeral &ephemeral)
{
        if (obj.count("events") == 0)
                return;

        auto events = obj.at("events");

        for (auto event : events) {
                auto type = event.at("type").get<std::string>();

                if (type == "m.typing") {
                        auto content     = event.at("content");
                        ephemeral.typing = content.at("user_ids").get<std::vector<std::string>>();
                } else if (type == "m.receipt") {
                        std::map<std::string, std::map<std::string, uint64_t>> receipts;

                        const auto content = event.at("content");

                        for (auto it = content.begin(); it != content.end(); ++it) {
                                std::map<std::string, uint64_t> user_times;

                                if (it.value().count("m.read") == 0)
                                        continue;

                                auto event_id = it.key();
                                auto users    = it.value().at("m.read");

                                for (auto uit = users.begin(); uit != users.end(); ++uit)
                                        user_times.emplace(uit.key(),
                                                           uit.value().at("ts").get<uint64_t>());

                                receipts.emplace(event_id, user_times);
                        }

                        ephemeral.receipts = receipts;
                }
        }
}

void
from_json(const json &obj, JoinedRoom &room)
{
        if (obj.count("state") != 0) {
                room.state = obj.at("state").get<State>();
        }
        if (obj.count("timeline") != 0) {
                room.timeline = obj.at("timeline").get<Timeline>();
        }

        if (obj.find("unread_notifications") != obj.end())
                room.unread_notifications =
                  obj.at("unread_notifications").get<UnreadNotifications>();

        if (obj.find("ephemeral") != obj.end())
                room.ephemeral = obj.at("ephemeral").get<Ephemeral>();
}

void
from_json(const json &obj, LeftRoom &room)
{
        if (obj.count("state") != 0) {
                room.state = obj.at("state").get<State>();
        }
        if (obj.count("timeline") != 0) {
                room.timeline = obj.at("timeline").get<Timeline>();
        }
}

std::string
InvitedRoom::name() const
{
        using Name   = mtx::events::StrippedEvent<mtx::events::state::Name>;
        using Member = mtx::events::StrippedEvent<mtx::events::state::Member>;

        std::string room_name;
        std::string member_name;

        for (const auto &event : invite_state) {
                if (mpark::holds_alternative<Name>(event)) {
                        room_name = mpark::get<Name>(event).content.name;
                } else if (mpark::holds_alternative<Member>(event)) {
                        if (member_name.empty())
                                member_name = mpark::get<Member>((event)).content.display_name;
                }
        }

        if (room_name.empty())
                return member_name;

        return room_name;
}

std::string
InvitedRoom::avatar() const
{
        using Avatar = mtx::events::StrippedEvent<mtx::events::state::Avatar>;
        using Member = mtx::events::StrippedEvent<mtx::events::state::Member>;

        std::string room_avatar;
        std::string member_avatar;

        for (const auto &event : invite_state) {
                if (mpark::holds_alternative<Avatar>(event)) {
                        auto msg    = mpark::get<Avatar>(event);
                        room_avatar = msg.content.url;
                } else if (mpark::holds_alternative<Member>(event)) {
                        auto msg = mpark::get<Member>(event);

                        // Pick the first avatar.
                        if (member_avatar.empty())
                                member_avatar = msg.content.avatar_url;
                }
        }

        if (room_avatar.empty())
                return member_avatar;

        return room_avatar;
}

void
from_json(const json &obj, InvitedRoom &room)
{
        utils::parse_stripped_events(obj.at("invite_state").at("events"), room.invite_state);
}

void
from_json(const json &obj, Rooms &rooms)
{
        if (obj.count("join") != 0) {
                auto joined = obj.at("join");

                for (auto it = joined.begin(); it != joined.end(); ++it)
                        rooms.join[it.key()] = it.value().get<JoinedRoom>();
        }

        if (obj.count("leave") != 0) {
                auto leave = obj.at("leave");

                for (auto it = leave.begin(); it != leave.end(); ++it)
                        rooms.leave[it.key()] = it.value().get<LeftRoom>();
        }

        if (obj.count("invite") != 0) {
                auto invite = obj.at("invite");

                for (auto it = invite.begin(); it != invite.end(); ++it)
                        rooms.invite[it.key()] = it.value().get<InvitedRoom>();
        }
}

void
from_json(const json &obj, Sync &response)
{
        if (obj.count("rooms") != 0) {
                response.rooms = obj.at("rooms").get<Rooms>();
        }
        response.next_batch = obj.at("next_batch").get<std::string>();
}
}
}
