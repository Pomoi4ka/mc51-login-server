#ifndef DATA_CLASSES_HPP_
#define DATA_CLASSES_HPP_

#include "BufStream.hpp"

// Format source: http://web.archive.org/web/20121102215436/http://wiki.vg/Slot_Data
class Slot {
public:
    // i don't see reasons to access those fields through
    // methods, i'll just make them public
    short itemID;
    int8_t itemCount;
    short itemDamage;
    // TODO: create separate interface for gzipped
    // nbt tags (not sure if it really needed)
    std::vector<uint8_t> tags;
    
    Slot(int itemID, int count = 1, int damage = 0, std::vector<uint8_t> tags = {});
    Slot(BufStream& stream);
    Slot();
    void send(BufStream& stream) const;
};

// Format source: http://web.archive.org/web/20130118192941/http://www.wiki.vg/Entities#Entity_Metadata_Format

// I won't implement this interface, i'll just transmit it as it is
class Metadata {
public:
    std::vector<uint8_t> data;
    Metadata(BufStream& stream);
    Metadata();
    void send(BufStream& stream) const;
};

// I don't like how it written btw
// Format source: http://web.archive.org/web/20130326024411/http://wiki.vg/Object_Data
class ObjectData {
public:
    int magicInt;
    struct {
        short x, y, z;
    } speed;

    ObjectData();
    ObjectData(BufStream& stream);
    void send(BufStream& stream) const;
};

#endif // DATA_CLASSES_HPP_
