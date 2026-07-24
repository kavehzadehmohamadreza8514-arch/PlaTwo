#include "NetworkPacket.h"

using namespace std;

NetworkPacket::NetworkPacket()
    : type(PacketType::ERROR_MSG), sender(""), data("") {
}

NetworkPacket::NetworkPacket(PacketType t, const string& senderUser, const string& d)
    : type(t), sender(senderUser), data(d) {
}

PacketType NetworkPacket::getType() const {
    return type;
}

string NetworkPacket::getSender() const {
    return sender;
}

string NetworkPacket::getData() const {
    return data;
}

string NetworkPacket::serialize() const {
    stringstream ss;
    ss << static_cast<int>(type) << "|" << sender << "|" << data << "\n";
    return ss.str();
}

NetworkPacket NetworkPacket::deserialize(const string& rawData) {
    stringstream ss(rawData);
    string typeStr, senderUser, payload;

    if (getline(ss, typeStr, '|') &&
        getline(ss, senderUser, '|') &&
        getline(ss, payload)) {

        if (!payload.empty() && payload.back() == '\r') payload.pop_back();

        int tInt = stoi(typeStr);
        return NetworkPacket(static_cast<PacketType>(tInt), senderUser, payload);
    }

    return NetworkPacket(PacketType::ERROR_MSG, "System", "Invalid Packet Format");
}