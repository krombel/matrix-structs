#include <string>

#include "mtx/events/create.hpp"
#include "mtx/macros.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, Create &create)
{
        if (is_spec_violation(obj, "creator", "m.room.create"))
                return;

        using namespace mtx::identifiers;
        create.creator = parse<User>(obj.at("creator").get<std::string>());

        if (obj.find("m.federate") != obj.end())
                create.federate = obj.at("m.federate").get<bool>();
}

void
to_json(json &obj, const Create &create)
{
        obj["creator"]    = create.creator.toString();
        obj["m.federate"] = create.federate;
}

} // namespace state
} // namespace events
} // namespace mtx
