#pragma once

#include <json.hpp>

#include "mtx/macros.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {

enum class EventType
{
        /// m.room.aliases
        RoomAliases,
        /// m.room.avatar
        RoomAvatar,
        /// m.room.canonical_alias
        RoomCanonicalAlias,
        /// m.room.create
        RoomCreate,
        /// m.room.guest_access
        RoomGuestAccess,
        /// m.room.history_visibility
        RoomHistoryVisibility,
        /// m.room.join_rules
        RoomJoinRules,
        /// m.room.member
        RoomMember,
        /// m.room.message
        RoomMessage,
        /// m.room.name
        RoomName,
        /// m.room.power_levels
        RoomPowerLevels,
        /// m.room.topic
        RoomTopic,
        /// m.room.redaction
        RoomRedaction,
        /// m.room.pinned_events
        RoomPinnedEvents,
        // Unsupported event
        Unsupported,
};

EventType
getEventType(const std::string &type);

EventType
getEventType(const json &obj);

template<class Content>
struct Event
{
        Content content;
        EventType type;
};

template<class Content>
void
to_json(json &obj, const Event<Content> &event)
{
        obj["content"] = event.content;

        switch (event.type) {
        case EventType::RoomAliases:
                obj["type"] = "m.room.aliases";
                break;
        case EventType::RoomAvatar:
                obj["type"] = "m.room.avatar";
                break;
        case EventType::RoomCanonicalAlias:
                obj["type"] = "m.room.canonical_alias";
                break;
        case EventType::RoomCreate:
                obj["type"] = "m.room.create";
                break;
        case EventType::RoomGuestAccess:
                obj["type"] = "m.room.guest_access";
                break;
        case EventType::RoomHistoryVisibility:
                obj["type"] = "m.room.history_visibility";
                break;
        case EventType::RoomJoinRules:
                obj["type"] = "m.room.join_rules";
                break;
        case EventType::RoomMember:
                obj["type"] = "m.room.member";
                break;
        case EventType::RoomMessage:
                obj["type"] = "m.room.message";
                break;
        case EventType::RoomName:
                obj["type"] = "m.room.name";
                break;
        case EventType::RoomPowerLevels:
                obj["type"] = "m.room.power_levels";
                break;
        case EventType::RoomTopic:
                obj["type"] = "m.room.topic";
                break;
        case EventType::RoomRedaction:
                obj["type"] = "m.room.redaction";
                break;
        case EventType::RoomPinnedEvents:
                obj["type"] = "m.room.pinned_events";
                break;
        case EventType::Unsupported:
                std::cout << "Unsupported type to serialize" << std::endl;
                break;
        }
}

template<class Content>
void
from_json(const json &obj, Event<Content> &event)
{
        if (is_spec_violation(obj, "content", "Event") || is_spec_violation(obj, "type", "Event"))
                return;

        try {
                event.content = obj.at("content").get<Content>();
                event.type    = getEventType(obj.at("type").get<std::string>());
        } catch (std::exception &e) {
                std::cout << "parse error: " << e.what() << obj << std::endl;
        }
}

template<class Content>
struct UnsignedData
{
        uint64_t age = 0;
        std::string transaction_id;
        std::string prev_sender;
        std::string replaces_state;

        Content prev_content;
};

template<class Content>
void
from_json(const json &obj, UnsignedData<Content> &data)
{
        if (obj.find("age") != obj.end())
                data.age = obj.at("age").get<uint64_t>();

        if (obj.find("transaction_id") != obj.end())
                data.transaction_id = obj.at("transaction_id").get<std::string>();

        if (obj.find("prev_sender") != obj.end())
                data.prev_sender = obj.at("prev_sender").get<std::string>();

        if (obj.find("replaces_state") != obj.end())
                data.replaces_state = obj.at("replaces_state").get<std::string>();

        if (obj.find("prev_content") != obj.end())
                data.prev_content = obj.at("prev_content").get<Content>();
}

template<class Content>
void
to_json(json &obj, const UnsignedData<Content> &event)
{
        if (!event.prev_sender.empty())
                obj["prev_sender"] = event.prev_sender;

        if (!event.transaction_id.empty())
                obj["transaction_id"] = event.transaction_id;

        if (!event.replaces_state.empty())
                obj["replaces_state"] = event.replaces_state;

        if (event.age != 0)
                obj["age"] = event.age;

        obj["prev_content"] = event.prev_content;
}

template<class Content>
struct StrippedEvent : public Event<Content>
{
        std::string sender;
        std::string state_key;
};

template<class Content>
void
from_json(const json &obj, StrippedEvent<Content> &event)
{
        Event<Content> base_event = event;
        from_json(obj, base_event);

        event.content = base_event.content;
        event.type    = base_event.type;

        if (is_spec_violation(obj, "sender", "StrippedEvent") ||
            is_spec_violation(obj, "state_key", "StrippedEvent"))
                return;

        event.sender    = obj.at("sender").get<std::string>();
        event.state_key = obj.at("state_key").get<std::string>();
}

template<class Content>
void
to_json(json &obj, const StrippedEvent<Content> &event)
{
        Event<Content> base_event = event;
        to_json(obj, base_event);

        obj["sender"]    = event.sender;
        obj["state_key"] = event.state_key;
}

template<class Content>
struct RoomEvent : public Event<Content>
{
        std::string event_id;
        std::string room_id;
        std::string sender;

        uint64_t origin_server_ts;

        // SPEC_BUG: The contents of unsigned_data are also present as top level keys.
        UnsignedData<Content> unsigned_data;
};

template<class Content>
void
from_json(const json &obj, RoomEvent<Content> &event)
{
        Event<Content> base_event = event;
        from_json(obj, base_event);

        event.content = base_event.content;
        event.type    = base_event.type;

        if (is_spec_violation(obj, "event_id", "RoomEvent") ||
            is_spec_violation(obj, "sender", "RoomEvent") ||
            is_spec_violation(obj, "origin_server_ts", "RoomEvent"))
                return;

        event.event_id         = obj.at("event_id").get<std::string>();
        event.sender           = obj.at("sender").get<std::string>();
        event.origin_server_ts = obj.at("origin_server_ts").get<double>();

        // SPEC_BUG: Not present in the state array returned by /sync.
        if (obj.find("room_id") != obj.end())
                event.room_id = obj.at("room_id").get<std::string>();

        if (obj.find("unsigned") != obj.end())
                event.unsigned_data = obj.at("unsigned").get<UnsignedData<Content>>();
}

template<class Content>
void
to_json(json &obj, const RoomEvent<Content> &event)
{
        Event<Content> base_event = event;
        to_json(obj, base_event);

        if (!event.room_id.empty())
                obj["room_id"] = event.room_id;

        obj["event_id"]         = event.event_id;
        obj["sender"]           = event.sender;
        obj["unsigned"]         = event.unsigned_data;
        obj["origin_server_ts"] = event.origin_server_ts;
}

template<class Content>
struct StateEvent : public RoomEvent<Content>
{
        std::string state_key;
        Content prev_content;
};

template<class Content>
void
to_json(json &obj, const StateEvent<Content> &event)
{
        RoomEvent<Content> base_event = event;
        to_json(obj, base_event);

        obj["state_key"]    = event.state_key;
        obj["prev_content"] = event.prev_content;
}

template<class Content>
void
from_json(const json &obj, StateEvent<Content> &event)
{
        RoomEvent<Content> base_event = event;
        from_json(obj, base_event);

        event.content          = base_event.content;
        event.type             = base_event.type;
        event.event_id         = base_event.event_id;
        event.room_id          = base_event.room_id;
        event.sender           = base_event.sender;
        event.origin_server_ts = base_event.origin_server_ts;
        event.unsigned_data    = base_event.unsigned_data;

        if (is_spec_violation(obj, "state_key", "StateEvent"))
                return;

        event.state_key = obj.at("state_key").get<std::string>();

        if (obj.find("prev_content") != obj.end())
                event.prev_content = obj.at("prev_content").get<Content>();
}

enum class MessageType
{
        // m.audio
        Audio,
        // m.emote
        Emote,
        // m.file
        File,
        // m.image
        Image,
        // m.location
        Location,
        // m.notice
        Notice,
        // m.text
        Text,
        // m.video
        Video,
        // Unrecognized message type
        Unknown,
};

MessageType
getMessageType(const std::string &type);

MessageType
getMessageType(const json &obj);

} // namespace events
} // namespace mtx
