#pragma once
#ifndef NETWORKPACKET_H
#define NETWORKPACKET_H

#include <string>
#include <sstream>

enum class PacketType {
    CONNECT_REQ,       
    CREATE_ROOM,        
    JOIN_ROOM,          
    ROOM_JOINED,        
    ERROR_MSG,          

    GAME_START,         
    TURN_CHANGE,        
    TIME_UP,            
    GAME_OVER,          

    MOVE_DOTS_BOXES,    
    MOVE_NINE_MENS,     
    MOVE_FANORONA,      

    PAUSE_SAVE_REQ,     
    RECONNECT_REQ       
};

class NetworkPacket {
private:
    PacketType type;
    std::string sender;
    std::string data; 

public:
    NetworkPacket();
    NetworkPacket(PacketType t, const std::string& senderUser, const std::string& d);

    PacketType getType() const;
    std::string getSender() const;
    std::string getData() const;

    std::string serialize() const;
    static NetworkPacket deserialize(const std::string& rawData);
};

#endif 