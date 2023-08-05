#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define ARRAY_LEN(a) (sizeof(a)/sizeof(*a))

#define BUFSTREAM_CLASSNAME "BufStream"

typedef enum {
    PACKET_KEEPALIVE = 0x00,
    PACKET_LOGIN,
    PACKET_HANDSHAKE,
    PACKET_CHAT_MESSAGE,
    PACKET_TIME_UPDATE,
    PACKET_ENTITY_EQUIPMENT,
    PACKET_SPAWN_POSITION,
    PACKET_USE_ENTITY,
    PACKET_UPDATE_HEALTH,
    PACKET_RESPAWN,
    PACKET_PLAYER,
    PACKET_PLAYER_POSITION,
    PACKET_PLAYER_LOOK,
    PACKET_PLAYER_POSITION_AND_LOOK,
    PACKET_PLAYER_DIGGING,
    PACKET_PLAYER_BLOCK_PLACEMENT,
    PACKET_HELD_ITEM_CHANGE,
    PACKET_USE_BED,
    PACKET_ANIMATION,
    PACKET_ENTITY_ACTION,
    PACKET_SPAWN_NAMED_ENTITY,
    PACKET_COLLECT_ITEM,
    PACKET_SPWAN_OBJECT,
    PACKET_SPAWN_MOB,
    PACKET_SPAWN_PAINTING,
    PACKET_SPAWN_EXPERIENCE_ORB,
    PACKET_ENTITY_VELOCITY,
    PACKET_DESTROY_ENTITY,
    PACKET_ENTITY,
    PACKET_RELATIVE_MOVE,
    PACKET_ENTITY_LOOK,
    PACKET_ENTITY_LOOK_AND_RELATIVE_MOVE,
    PACKET_ENTITY_TELEPORT,
    PACKET_ENTITY_HEAD_LOOK,
    PACKET_ENTITY_STATUS,
    PACKET_ATTACH_ENTITY,
    PACKET_ENTITY_METADATA,
    PACKET_ENTITY_EFFECT,
    PACKET_REMOVE_ENTITY_EFFECT,
    PACKET_SET_EXPERIENCE,
    PACKET_CHUNK_DATA,
    PACKET_MULTIBLOCK_CHANGE,
    PACKET_BLOCK_CHANGE,
    PACKET_BLOCK_ACTION,
    PACKET_BLOCK_ANIMATION,
    PACKET_BLOCK_BREAK_ANIMATION,
    PACKET_MAP_CHUNK_BULK,
    PACKET_EXPLOSION,
    PACKET_SOUND_OR_PARTICLE,
    PACKET_NAMED_SOUND_EFFECT,
    PACKET_CHANGE_GAME_STATE,
    PACKET_SPAWN_GLOBAL_ENTITY,
    PACKET_OPEN_WINDOW = 0x64,
    PACKET_CLOSE_WINDOW,
    PACKET_CLICK_WINDOW,
    PACKET_SET_SLOT,
    PACKET_SET_WINDOW_ITEMS,
    PACKET_UPDATE_WINDOW_PROPERTY,
    PACKET_CONFIRM_TRANSACTION,
    PACKET_CREATIVE_INVENTORY_ACTION,
    PACKET_ENCHANT_ITEM,
    PACKET_UPDATE_SIGN = 0x82,
    PACKET_ITEM_DATA,
    PACKET_UPDATE_TILE_ENTITY,
    PACKET_INCREMENT_STATISTIC = 0xc8,
    PACKET_PLAYER_LIST_ITEM,
    PACKET_PLAYER_ABILITIES,
    PACKET_TAB_COMPLETE,
    PACKET_CLIENT_SETTINGS,
    PACKET_CLIENT_STATUSES,
    PACKET_PLUGIN_MESSAGE = 0xfa,
    PACKET_ENCRYPTION_KEY_RESPONSE = 0xfc,
    PACKET_ENCRYPTION_KEY_REQUEST,
    PACKET_SERVER_LIST_PING,
    PACKET_DISCONNECT
} PacketID;

const int packetIds[] = {
    PACKET_KEEPALIVE,
    PACKET_LOGIN,
    PACKET_HANDSHAKE,
    PACKET_CHAT_MESSAGE,
    PACKET_TIME_UPDATE,
    PACKET_ENTITY_EQUIPMENT,
    PACKET_SPAWN_POSITION,
    PACKET_USE_ENTITY,
    PACKET_UPDATE_HEALTH,
    PACKET_RESPAWN,
    PACKET_PLAYER,
    PACKET_PLAYER_POSITION,
    PACKET_PLAYER_LOOK,
    PACKET_PLAYER_POSITION_AND_LOOK,
    PACKET_PLAYER_DIGGING,
    PACKET_PLAYER_BLOCK_PLACEMENT,
    PACKET_HELD_ITEM_CHANGE,
    PACKET_USE_BED,
    PACKET_ANIMATION,
    PACKET_ENTITY_ACTION,
    PACKET_SPAWN_NAMED_ENTITY,
    PACKET_COLLECT_ITEM,
    PACKET_SPWAN_OBJECT,
    PACKET_SPAWN_MOB,
    PACKET_SPAWN_PAINTING,
    PACKET_SPAWN_EXPERIENCE_ORB,
    PACKET_ENTITY_VELOCITY,
    PACKET_DESTROY_ENTITY,
    PACKET_ENTITY,
    PACKET_RELATIVE_MOVE,
    PACKET_ENTITY_LOOK,
    PACKET_ENTITY_LOOK_AND_RELATIVE_MOVE,
    PACKET_ENTITY_TELEPORT,
    PACKET_ENTITY_HEAD_LOOK,
    PACKET_ENTITY_STATUS,
    PACKET_ATTACH_ENTITY,
    PACKET_ENTITY_METADATA,
    PACKET_ENTITY_EFFECT,
    PACKET_REMOVE_ENTITY_EFFECT,
    PACKET_SET_EXPERIENCE,
    PACKET_CHUNK_DATA,
    PACKET_MULTIBLOCK_CHANGE,
    PACKET_BLOCK_CHANGE,
    PACKET_BLOCK_ACTION,
    PACKET_BLOCK_ANIMATION,
    PACKET_BLOCK_BREAK_ANIMATION,
    PACKET_MAP_CHUNK_BULK,
    PACKET_EXPLOSION,
    PACKET_SOUND_OR_PARTICLE,
    PACKET_NAMED_SOUND_EFFECT,
    PACKET_CHANGE_GAME_STATE,
    PACKET_SPAWN_GLOBAL_ENTITY,
    PACKET_OPEN_WINDOW,
    PACKET_CLOSE_WINDOW,
    PACKET_CLICK_WINDOW,
    PACKET_SET_SLOT,
    PACKET_SET_WINDOW_ITEMS,
    PACKET_UPDATE_WINDOW_PROPERTY,
    PACKET_CONFIRM_TRANSACTION,
    PACKET_CREATIVE_INVENTORY_ACTION,
    PACKET_ENCHANT_ITEM,
    PACKET_UPDATE_SIGN,
    PACKET_ITEM_DATA,
    PACKET_UPDATE_TILE_ENTITY,
    PACKET_INCREMENT_STATISTIC,
    PACKET_PLAYER_LIST_ITEM,
    PACKET_PLAYER_ABILITIES,
    PACKET_TAB_COMPLETE,
    PACKET_CLIENT_SETTINGS,
    PACKET_CLIENT_STATUSES,
    PACKET_PLUGIN_MESSAGE,
    PACKET_ENCRYPTION_KEY_RESPONSE,
    PACKET_ENCRYPTION_KEY_REQUEST,
    PACKET_SERVER_LIST_PING,
    PACKET_DISCONNECT,
};

const char *(classNames[]) = {
    [PACKET_KEEPALIVE]                     = "PacketKeepalive",
    [PACKET_LOGIN]                         = "PacketLogin",
    [PACKET_HANDSHAKE]                     = "PacketHandshake",
    [PACKET_CHAT_MESSAGE]                  = "PacketChatMessage",
    [PACKET_TIME_UPDATE]                   = "PacketTimeUpdate",
    [PACKET_ENTITY_EQUIPMENT]              = "PacketEntityEquipment",
    [PACKET_SPAWN_POSITION]                = "PacketSpawnPosition",
    [PACKET_USE_ENTITY]                    = "PacketUseEntity",
    [PACKET_UPDATE_HEALTH]                 = "PacketUpdateHealth",
    [PACKET_RESPAWN]                       = "PacketRespawn",
    [PACKET_PLAYER]                        = "PacketPlayer",
    [PACKET_PLAYER_POSITION]               = "PacketPlayerPosition",
    [PACKET_PLAYER_LOOK]                   = "PacketPlayerLook",
    [PACKET_PLAYER_POSITION_AND_LOOK]      = "PacketPlayerPositionAndLook",
    [PACKET_PLAYER_DIGGING]                = "PacketPlayerDigging",
    [PACKET_PLAYER_BLOCK_PLACEMENT]        = "PacketPlayerBlockPlacement",
    [PACKET_HELD_ITEM_CHANGE]              = "PacketHeldItemChange",
    [PACKET_USE_BED]                       = "PacketUseBed",
    [PACKET_ANIMATION]                     = "PacketAnimation",
    [PACKET_ENTITY_ACTION]                 = "PacketEntityAction",
    [PACKET_SPAWN_NAMED_ENTITY]            = "PacketSpawnNamedEntity",
    [PACKET_COLLECT_ITEM]                  = "PacketCollectItem",
    [PACKET_SPWAN_OBJECT]                  = "PacketSpwanObject",
    [PACKET_SPAWN_MOB]                     = "PacketSpawnMob",
    [PACKET_SPAWN_PAINTING]                = "PacketSpawnPainting",
    [PACKET_SPAWN_EXPERIENCE_ORB]          = "PacketSpawnExperienceOrb",
    [PACKET_ENTITY_VELOCITY]               = "PacketEntityVelocity",
    [PACKET_DESTROY_ENTITY]                = "PacketDestroyEntity",
    [PACKET_ENTITY]                        = "PacketEntity",
    [PACKET_RELATIVE_MOVE]                 = "PacketRelativeMove",
    [PACKET_ENTITY_LOOK]                   = "PacketEntityLook",
    [PACKET_ENTITY_LOOK_AND_RELATIVE_MOVE] = "PacketEntityLookAndRelativeMove",
    [PACKET_ENTITY_TELEPORT]               = "PacketEntityTeleport",
    [PACKET_ENTITY_HEAD_LOOK]              = "PacketEntityHeadLook",
    [PACKET_ENTITY_STATUS]                 = "PacketEntityStatus",
    [PACKET_ATTACH_ENTITY]                 = "PacketAttachEntity",
    [PACKET_ENTITY_METADATA]               = "PacketEntityMetadata",
    [PACKET_ENTITY_EFFECT]                 = "PacketEntityEffect",
    [PACKET_REMOVE_ENTITY_EFFECT]          = "PacketRemoveEntityEffect",
    [PACKET_SET_EXPERIENCE]                = "PacketSetExperience",
    [PACKET_CHUNK_DATA]                    = "PacketChunkData",
    [PACKET_MULTIBLOCK_CHANGE]             = "PacketMultiblockChange",
    [PACKET_BLOCK_CHANGE]                  = "PacketBlockChange",
    [PACKET_BLOCK_ACTION]                  = "PacketBlockAction",
    [PACKET_BLOCK_ANIMATION]               = "PacketBlockAnimation",
    [PACKET_BLOCK_BREAK_ANIMATION]         = "PacketBlockBreakAnimation",
    [PACKET_MAP_CHUNK_BULK]                = "PacketMapChunkBulk",
    [PACKET_EXPLOSION]                     = "PacketExplosion",
    [PACKET_SOUND_OR_PARTICLE]             = "PacketSoundOrParticle",
    [PACKET_NAMED_SOUND_EFFECT]            = "PacketNamedSoundEffect",
    [PACKET_CHANGE_GAME_STATE]             = "PacketChangeGameState",
    [PACKET_SPAWN_GLOBAL_ENTITY]           = "PacketSpawnGlobalEntity",
    [PACKET_OPEN_WINDOW]                   = "PacketOpenWindow",
    [PACKET_CLOSE_WINDOW]                  = "PacketCloseWindow",
    [PACKET_CLICK_WINDOW]                  = "PacketClickWindow",
    [PACKET_SET_SLOT]                      = "PacketSetSlot",
    [PACKET_SET_WINDOW_ITEMS]              = "PacketSetWindowItems",
    [PACKET_UPDATE_WINDOW_PROPERTY]        = "PacketUpdateWindowProperty",
    [PACKET_CONFIRM_TRANSACTION]           = "PacketConfirmTransaction",
    [PACKET_CREATIVE_INVENTORY_ACTION]     = "PacketCreativeInventoryAction",
    [PACKET_ENCHANT_ITEM]                  = "PacketEnchantItem",
    [PACKET_UPDATE_SIGN]                   = "PacketUpdateSign",
    [PACKET_ITEM_DATA]                     = "PacketItemData",
    [PACKET_UPDATE_TILE_ENTITY]            = "PacketUpdateTileEntity",
    [PACKET_INCREMENT_STATISTIC]           = "PacketIncrementStatistic",
    [PACKET_PLAYER_LIST_ITEM]              = "PacketPlayerListItem",
    [PACKET_PLAYER_ABILITIES]              = "PacketPlayerAbilities",
    [PACKET_TAB_COMPLETE]                  = "PacketTabComplete",
    [PACKET_CLIENT_SETTINGS]               = "PacketClientSettings",
    [PACKET_CLIENT_STATUSES]               = "PacketClientStatuses",
    [PACKET_PLUGIN_MESSAGE]                = "PacketPluginMessage",
    [PACKET_ENCRYPTION_KEY_RESPONSE]       = "PacketEncryptionKeyResponse",
    [PACKET_ENCRYPTION_KEY_REQUEST]        = "PacketEncryptionKeyRequest",
    [PACKET_SERVER_LIST_PING]              = "PacketServerListPing",
    [PACKET_DISCONNECT]                    = "PacketDisconnect",
};

typedef struct {
    enum {
        BYTE,
        BOOLEAN,
        SHORT,
        FLOAT,
        INT,
        DOUBLE,
        LONG,
        STRING,
        SLOT,
        BYTEARRAY,
        END,
    } type;
    const char *name;
} Field;

const char* fieldtype_to_cstr(const Field* f)
{
    switch (f->type) {
    case BYTE: return "uint8_t";
    case SHORT: return "short";
    case FLOAT: return "float";
    case INT: return "int";
    case DOUBLE: return "double";
    case LONG: return "long";
    case STRING: return "std::string";
    case BYTEARRAY: return "std::vector<uint8_t>";
    case BOOLEAN: return "bool";
    case SLOT: return "Packets::slot_t";
    }
    fprintf(stderr, "ERROR: %d is not implemented\n", f->type);
    assert(0);
}

const Field *(fields[]) = {
    [PACKET_KEEPALIVE]                     = (Field[]){{INT, "keepaliveID"}, {END}},
    [PACKET_LOGIN]                         = (Field[]){{INT, "entityID"}, {STRING, "levelType"},
                                              {BYTE, "gamemode"}, {BYTE, "dimension"},
                                              {BYTE, NULL}, {BYTE, "maxPlayers"}, {END}},
    [PACKET_HANDSHAKE]                     = (Field[]){{BYTE, "protocolVer"}, {STRING, "username"},
                                              {STRING, "serverHost"}, {INT, "serverPort"}, {END}},
    [PACKET_CHAT_MESSAGE]                  = (Field[]){{STRING, "message"}, {END}},
    [PACKET_TIME_UPDATE]                   = (Field[]){{LONG, "ageOfWorld"}, {LONG, "timeOfDay"}, {END}},
    [PACKET_ENTITY_EQUIPMENT]              = (Field[]){{INT, "entityID"}, {SHORT, "slot"}, {SLOT, "item"}, {END}},
    [PACKET_SPAWN_POSITION]                = (Field[]){{INT, "x"}, {INT, "y"}, {INT, "z"}, {END}},
    [PACKET_USE_ENTITY]                    = (Field[]){{INT, "user"}, {INT, "target"}, {BOOLEAN, "mouseButton"}, {END}},
    [PACKET_UPDATE_HEALTH]                 = (Field[]){{END}},
    [PACKET_RESPAWN]                       = (Field[]){{END}},
    [PACKET_PLAYER]                        = (Field[]){{END}},
    [PACKET_PLAYER_POSITION]               = (Field[]){{END}},
    [PACKET_PLAYER_LOOK]                   = (Field[]){{END}},
    [PACKET_PLAYER_POSITION_AND_LOOK]      = (Field[]){{END}},
    [PACKET_PLAYER_DIGGING]                = (Field[]){{END}},
    [PACKET_PLAYER_BLOCK_PLACEMENT]        = (Field[]){{END}},
    [PACKET_HELD_ITEM_CHANGE]              = (Field[]){{END}},
    [PACKET_USE_BED]                       = (Field[]){{END}},
    [PACKET_ANIMATION]                     = (Field[]){{END}},
    [PACKET_ENTITY_ACTION]                 = (Field[]){{END}},
    [PACKET_SPAWN_NAMED_ENTITY]            = (Field[]){{END}},
    [PACKET_COLLECT_ITEM]                  = (Field[]){{END}},
    [PACKET_SPWAN_OBJECT]                  = (Field[]){{END}},
    [PACKET_SPAWN_MOB]                     = (Field[]){{END}},
    [PACKET_SPAWN_PAINTING]                = (Field[]){{END}},
    [PACKET_SPAWN_EXPERIENCE_ORB]          = (Field[]){{END}},
    [PACKET_ENTITY_VELOCITY]               = (Field[]){{END}},
    [PACKET_DESTROY_ENTITY]                = (Field[]){{END}},
    [PACKET_ENTITY]                        = (Field[]){{END}},
    [PACKET_RELATIVE_MOVE]                 = (Field[]){{END}},
    [PACKET_ENTITY_LOOK]                   = (Field[]){{END}},
    [PACKET_ENTITY_LOOK_AND_RELATIVE_MOVE] = (Field[]){{END}},
    [PACKET_ENTITY_TELEPORT]               = (Field[]){{END}},
    [PACKET_ENTITY_HEAD_LOOK]              = (Field[]){{END}},
    [PACKET_ENTITY_STATUS]                 = (Field[]){{END}},
    [PACKET_ATTACH_ENTITY]                 = (Field[]){{END}},
    [PACKET_ENTITY_METADATA]               = (Field[]){{END}},
    [PACKET_ENTITY_EFFECT]                 = (Field[]){{END}},
    [PACKET_REMOVE_ENTITY_EFFECT]          = (Field[]){{END}},
    [PACKET_SET_EXPERIENCE]                = (Field[]){{END}},
    [PACKET_CHUNK_DATA]                    = (Field[]){{END}},
    [PACKET_MULTIBLOCK_CHANGE]             = (Field[]){{END}},
    [PACKET_BLOCK_CHANGE]                  = (Field[]){{END}},
    [PACKET_BLOCK_ACTION]                  = (Field[]){{END}},
    [PACKET_BLOCK_ANIMATION]               = (Field[]){{END}},
    [PACKET_BLOCK_BREAK_ANIMATION]         = (Field[]){{END}},
    [PACKET_MAP_CHUNK_BULK]                = (Field[]){{END}},
    [PACKET_EXPLOSION]                     = (Field[]){{END}},
    [PACKET_SOUND_OR_PARTICLE]             = (Field[]){{END}},
    [PACKET_NAMED_SOUND_EFFECT]            = (Field[]){{END}},
    [PACKET_CHANGE_GAME_STATE]             = (Field[]){{END}},
    [PACKET_SPAWN_GLOBAL_ENTITY]           = (Field[]){{END}},
    [PACKET_OPEN_WINDOW]                   = (Field[]){{END}},
    [PACKET_CLOSE_WINDOW]                  = (Field[]){{END}},
    [PACKET_CLICK_WINDOW]                  = (Field[]){{END}},
    [PACKET_SET_SLOT]                      = (Field[]){{END}},
    [PACKET_SET_WINDOW_ITEMS]              = (Field[]){{END}},
    [PACKET_UPDATE_WINDOW_PROPERTY]        = (Field[]){{END}},
    [PACKET_CONFIRM_TRANSACTION]           = (Field[]){{END}},
    [PACKET_CREATIVE_INVENTORY_ACTION]     = (Field[]){{END}},
    [PACKET_ENCHANT_ITEM]                  = (Field[]){{END}},
    [PACKET_UPDATE_SIGN]                   = (Field[]){{END}},
    [PACKET_ITEM_DATA]                     = (Field[]){{END}},
    [PACKET_UPDATE_TILE_ENTITY]            = (Field[]){{END}},
    [PACKET_INCREMENT_STATISTIC]           = (Field[]){{END}},
    [PACKET_PLAYER_LIST_ITEM]              = (Field[]){{END}},
    [PACKET_PLAYER_ABILITIES]              = (Field[]){{END}},
    [PACKET_TAB_COMPLETE]                  = (Field[]){{END}},
    [PACKET_CLIENT_SETTINGS]               = (Field[]){{END}},
    [PACKET_CLIENT_STATUSES]               = (Field[]){{END}},
    [PACKET_PLUGIN_MESSAGE]                = (Field[]){{END}},
    [PACKET_ENCRYPTION_KEY_RESPONSE]       = (Field[]){{END}},
    [PACKET_ENCRYPTION_KEY_REQUEST]        = (Field[]){{END}},
    [PACKET_SERVER_LIST_PING]              = (Field[]){{END}},
    [PACKET_DISCONNECT]                    = (Field[]){{END}},
};

char* shift(int *argc, char*** argv)
{
    if ((*argc)-- <= 0) return NULL;
    return *(*argv)++;
}

void generate_fields(FILE* out, int id)
{
    for (const Field* class_fields = fields[id]; class_fields->type != END; class_fields++) {
        if (class_fields->name == NULL) continue;
        fprintf(out, "    %s %s;\n", fieldtype_to_cstr(class_fields), class_fields->name);
    }
}

char* fields_list_to_cstr(const Field* fields)
{
    static char buffer[256];
    *buffer = 0;
    
    int pos = 0;
    const Field* f = fields;
    for (; f->type != END; f++) {
        if (f->name == NULL) continue;
        pos += snprintf(&buffer[pos], sizeof(buffer) - pos,
                        "%s %s", fieldtype_to_cstr(f),
                        f->name);
        if (f[1].type != END)
            pos += snprintf(&buffer[pos], sizeof(buffer) - pos, ", ");
    }
    return buffer;
}

void generate_methods_decl(FILE* out, int id)
{
    fprintf(out, "    %s(%s);\n",
            classNames[id], fields_list_to_cstr(fields[id]));
    fprintf(out, "    %s(" BUFSTREAM_CLASSNAME "& stream);\n",
            classNames[id]);
    fprintf(out, "    void send(" BUFSTREAM_CLASSNAME "& stream) const;\n");
}

char* io_func_prefix_for_type(const Field* f)
{
    static char buf[64];
    *buf = 0;
    
    switch (f->type) {
    case BYTE:
    case BOOLEAN:
    case SHORT:
    case FLOAT:
    case INT:
    case DOUBLE:
    case LONG: {
        snprintf(buf, sizeof(buf),
                 "be<%s>", fieldtype_to_cstr(f));
        return buf;
    }
    case STRING:
    case SLOT: {
        snprintf(buf, sizeof(buf),
                 "<%s>", fieldtype_to_cstr(f));
        return buf;
    }
    case BYTEARRAY:
        return "V<short, uint8_t>";
    }

    fprintf(stderr, "ERROR: not implemented for %d\n", f->type);
    assert(0);
}

void generate_class_decl(FILE* out, int id)
{
    fprintf(out, "class %s: Packet {\n", classNames[id]);
    fprintf(out, "public:\n");
    generate_fields(out, id);
    generate_methods_decl(out, id);
    fprintf(out, "};\n\n");
}

/* Standard methods as constructors (BufStream&; [fields]) and send method */ 
void generate_methods_defs_for_class(FILE* out, const char* namespace,
                                     const char* classname, int packetid, const Field* fields)
{
    fprintf(out, "%s::%s::%s(%s)\n{\n", namespace, classname, classname,
            fields_list_to_cstr(fields));
    for (const Field* f = fields; f->type != END; ++f) {
        if (f->name == NULL) continue;
        fprintf(out, "    this->%s = %s;\n", f->name, f->name);
    }
    fprintf(out, "}\n\n");

    fprintf(out, "%s::%s::%s(" BUFSTREAM_CLASSNAME "& stream)\n{\n", namespace, classname, classname,
            fields_list_to_cstr(fields));
    for (const Field* f = fields; f->type != END; ++f) {
        if (f->name == NULL)
            fprintf(out, "    stream.read%s();\n", io_func_prefix_for_type(f));
        else 
            fprintf(out, "    this->%s = stream.read%s();\n", f->name, io_func_prefix_for_type(f));
    }
    fprintf(out, "}\n\n");

    fprintf(out, "void %s::%s::send(" BUFSTREAM_CLASSNAME "& stream) const\n{\n",
            namespace, classname);
    if (packetid >= 0) {
        fprintf(out, "    stream.write<uint8_t>(0x%x);\n", packetid);
    }
    for (const Field* f = fields; f->type != END; ++f) {
        if (f->name == NULL)
            fprintf(out, "    stream.write%s(0);\n", io_func_prefix_for_type(f));
        else
            fprintf(out, "    stream.write%s(this->%s);\n", io_func_prefix_for_type(f), f->name);
    }
    fprintf(out, "}\n\n");
}

int main(int argc, char** argv)
{
    const char* decloutput = NULL, *defoutput = NULL;
    const char* progname = shift(&argc, &argv);
    while (true) {
        const char* arg = shift(&argc, &argv);
        const char* val;
        if (!arg) break;

        if (strcmp(arg, "-declo") == 0) {
            val = shift(&argc, &argv);
            if (!val) {
                fprintf(stderr, "ERROR: `%s` expected filename\n", arg);
                return 1;
            }
            decloutput = val;
        } else if (strcmp(arg, "-defo") == 0) {
            val = shift(&argc, &argv);
            if (!val) {
                fprintf(stderr, "ERROR: `%s` expected filename\n", arg);
                return 1;
            }
            defoutput = val;
        }else {
            fprintf(stderr, "ERROR: unknown argument `%s`\n", arg);
            return 1;
        }
    }

    FILE* out;
    if (decloutput)
         out = fopen(decloutput, "w");
    else out = stdout;

    fprintf(out, "#ifndef PACKETS_HPP_\n");
    fprintf(out, "#define PACKETS_HPP_\n\n");
    fprintf(out, "#include <cstdlib>\n");
    fprintf(out, "#include <cstdint>\n");
    fprintf(out, "#include <string>\n");
    fprintf(out, "#include <vector>\n\n");
    fprintf(out, "#include <BufStream.hpp>\n\n");
    fprintf(out, "namespace Packets {\n");

    fprintf(out,
            "class Packet {\n"
            "public:\n"
            "    virtual void send(" BUFSTREAM_CLASSNAME "& stream) = 0;\n"
            "};\n");
    
    fprintf(out,
            "\nclass slot_t {\n"
            "public:\n"
            "    short itemID;\n"
            "    int8_t count;\n"
            "    short damage;\n"
            "    std::vector<uint8_t> metadata;\n"
            "    slot_t(" BUFSTREAM_CLASSNAME "& stream);\n"
            "    slot_t();\n"
            "    void send(" BUFSTREAM_CLASSNAME "& stream) const;\n"
            "};\n\n");
    for (size_t i = 0; i < ARRAY_LEN(packetIds); ++i) {
        generate_class_decl(out, packetIds[i]);
    }
    fprintf(out, "}\n\n");
    fprintf(out, "#endif // PACKETS_HPP_\n");
    if (decloutput) fclose(out);
    if (defoutput) out = fopen(defoutput, "w");

    if (decloutput) {
        const char* fname = decloutput + strlen(decloutput);
        while (fname > decloutput && *fname != '/') fname--;
        fprintf(out, "#include <%s>\n\n", fname);
    }

    fprintf(out, "Packets::slot_t::slot_t()\n{\n"
            "    itemID = -1;\n"
            "    count = -1;\n"
            "    damage = -1;\n"
            "}\n\n"
            "Packets::slot_t::slot_t(" BUFSTREAM_CLASSNAME "& stream)\n{\n"
            "    itemID = stream.readbe<short>();\n"
            "    if (itemID > -1) {\n"
            "        count = stream.read<int8_t>();\n"
            "        damage = stream.readbe<int16_t>();\n"
            "        auto metadataLen = stream.readbe<int16_t>();\n"
            "        if (metadataLen > 0) {\n"
            "            metadata.resize(metadataLen);\n"
            "            stream.read(metadata.data(), metadataLen);\n"
            "        }\n"
            "    }\n"
            "}\n\n"
            "void Packets::slot_t::send(" BUFSTREAM_CLASSNAME "& stream) const\n{\n"
            "    stream.writebe<short>(itemID);\n"
            "    if (itemID < 0) return;\n"
            "    stream.writebe<int8_t>(count);\n"
            "    stream.writebe<int16_t>(damage);\n"
            "    if (metadata.size() == 0)\n"
            "        stream.writebe<int16_t>(-1);\n"
            "    else stream.writeV<int16_t>(metadata);\n"
            "}\n\n");
    
    for (size_t i = 0; i < ARRAY_LEN(packetIds); ++i) {
        generate_methods_defs_for_class
            (out, "Packets", classNames[packetIds[i]],
             packetIds[i], fields[packetIds[i]]);
    }
    
    fclose(out);
    return 0;
}
