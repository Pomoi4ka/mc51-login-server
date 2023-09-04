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

std::vector<uint8_t> Slot::to_bytes() const
{
    std::vector<uint8_t> result;
    auto push = [&](uint8_t* p, size_t s = 1) {
        for (size_t i = 0; i < s; ++i) {
            result.push_back(p[i]);
        }
    };

    auto pushbe = [&](uint8_t* p, size_t s = 1) {
        for (int i = s-1; i >= 0; --i) {
            result.push_back(p[i]);
        }
    };

    pushbe((uint8_t*)&itemID, sizeof(short));
    if (itemID < 0) return result;
    pushbe((uint8_t*)&itemCount);
    pushbe((uint8_t*)&itemDamage, sizeof(short));
    if (tags.size() == 0) {
        short tmp = -1;
        pushbe((uint8_t*)&tmp, sizeof(short));
        return result;
    }
    short tagsSize = tags.size();
    pushbe((uint8_t*)&tagsSize, sizeof(short));
    push((uint8_t*)tags.data(), tags.size());
    return result;
}

Metadata::Metadata(BufStream& stream):
    data()
{
    do {
        uint8_t b = stream.read<uint8_t>();
        data.push_back(b);
        if (b == 127) break;
        uint8_t type = (b >> 5u) & 0xffu;
        auto push = [&](size_t typesize) {
            for (size_t i = 0; i < typesize; ++i)
                data.push_back(stream.read<uint8_t>());
        };
        switch (type) {
        case 0: push(sizeof(uint8_t)); break;
        case 1: push(sizeof(uint16_t)); break;
        case 2: push(sizeof(uint32_t)); break;
        case 3: push(sizeof(uint32_t)); break;
        case 4: {
            auto len = stream.readbe<uint16_t>();
            data.push_back(((uint8_t*)&len)[1]);
            data.push_back(((uint8_t*)&len)[0]);
            for (size_t i = 0; i < len; ++i)
                push(sizeof(uint16_t));
        } break;
        case 5: {
            Slot s(stream);
            for (auto& i: s.to_bytes())
                data.push_back(i);
        } break;
        case 6: {
            push(sizeof(uint32_t)); break;
            push(sizeof(uint32_t)); break;
            push(sizeof(uint32_t)); break;
        } break;
        }
    } while (true);
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
