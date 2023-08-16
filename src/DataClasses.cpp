#include "DataClasses.hpp"

Slot::Slot(int itemID, int count, int damage, std::vector<uint8_t> tags):
    tags(tags)
{
    this->itemID = itemID;
    this->itemCount = count;
    this->itemDamage = damage;
}

Slot::Slot(BufStream& stream)
{
    itemID = stream.readbe<short>();
    if (itemID < 0) {
        itemCount = -1;
        itemDamage = -1;
    } else {
        itemCount = stream.read<int8_t>();
        itemDamage = stream.readbe<short>();
        tags = stream.readV<short, uint8_t>();
    }
}

Slot::Slot():
    tags()
{
    itemID = -1;
    itemCount = -1;
    itemDamage = -1;
}

void Slot::send(BufStream& stream) const
{
    stream.writebe<short>(itemID);
    if (itemID < 0) return;
    stream.write<int8_t>(itemCount);
    stream.writebe<short>(itemDamage);
    if (tags.size() == 0) {
        stream.writebe<short>(-1);
        return;
    }
    stream.writeV<short>(tags);
}

Metadata::Metadata(BufStream& stream)
{
    uint8_t b;
    do {
        b = stream.read<uint8_t>();
        data.push_back(b);
    } while (b != 127);
}

Metadata::Metadata():
    data()
{}

void Metadata::send(BufStream& stream) const
{
    stream.write(data.data(), data.size());
}

ObjectData::ObjectData()
{
    magicInt = 0;
}

ObjectData::ObjectData(BufStream& stream)
{
    magicInt = stream.readbe<int>();
    if (magicInt != 0) {
        speed.x = stream.readbe<short>();
        speed.y = stream.readbe<short>();
        speed.z = stream.readbe<short>();
    }
}

void ObjectData::send(BufStream& stream) const
{
    stream.writebe<int>(magicInt);
    if (magicInt == 0) return;
    stream.writebe<short>(speed.x);
    stream.writebe<short>(speed.y);
    stream.writebe<short>(speed.z);
}
