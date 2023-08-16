#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define ARRAY_LEN(a) (sizeof(a)/sizeof(*a))

#define BUFSTREAM_CLASSNAME "BufStream"
#define CHUNK_META_INFORMATION_STRUCT "ChunkMetaInformation"
#define EXPLOSION_RECORD_STRUCT "ExplosionRecord"

#define IS_TYPE_CLASS(x) (x == SLOT || x == BYTEARRAY || x == INTARRAY || x == METADATA || x == OBJECTDATA || x == STRING)
#define IS_TYPE_ARRAY(x) (x == INTARRAY || x == BYTEARRAY || x == STRING)

typedef struct {
    char data[512];
} TEMPStr;

#define TEMPORATE(x) (TEMPStr) { .data = x }

#define GENERATED_MARK \
"/* Don't touch this file. This file was generated by %s */"

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
    PACKET_SPAWN_OBJECT,
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
    PACKET_SPAWN_OBJECT,
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
    [PACKET_SPAWN_OBJECT]                  = "PacketSpawnObject",
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

typedef enum {
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
    BYTEARRAY4,
    BYTEARRAY_NAMED,
    INTARRAY,
    METADATA,
    OBJECTDATA,
    STRUCTARRAY,
    END,
} FieldType;

typedef struct Field {
    FieldType type;
    const char *name;
    const char* argName;
    const char* argName2;
} Field;

TEMPStr fieldtype_to_cstr(const Field* f)
{
    switch (f->type) {
    case BYTE:       return TEMPORATE("uint8_t");
    case SHORT:      return TEMPORATE("short");
    case FLOAT:      return TEMPORATE("float");
    case INT:        return TEMPORATE("int");
    case DOUBLE:     return TEMPORATE("double");
    case LONG:       return TEMPORATE("long");
    case STRING:     return TEMPORATE("std::string");
    case BYTEARRAY_NAMED:
    case BYTEARRAY4:
    case BYTEARRAY:  return TEMPORATE("std::vector<uint8_t>");
    case INTARRAY:   return TEMPORATE("std::vector<int>");
    case BOOLEAN:    return TEMPORATE("bool");
    case SLOT:       return TEMPORATE("Slot");
    case METADATA:   return TEMPORATE("Metadata");
    case OBJECTDATA: return TEMPORATE("ObjectData");
    case STRUCTARRAY: {
        TEMPStr str = {0};
        sprintf(str.data, "std::vector<%s>", f->argName);
        return str;
    } 
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
    [PACKET_UPDATE_HEALTH]                 = (Field[]){{SHORT, "health"}, {SHORT, "food"}, {FLOAT, "saturation"}, {END}},
    [PACKET_RESPAWN]                       = (Field[]){{INT, "dimension"}, {BYTE, "difficulty"}, {BYTE, "gamemode"},
                                                       {SHORT, "worldHeight"}, {STRING, "levelType"}, {END}},
    [PACKET_PLAYER]                        = (Field[]){{BOOLEAN, "onGround"}, {END}},
    [PACKET_PLAYER_POSITION]               = (Field[]){{DOUBLE, "x"}, {DOUBLE, "y"}, {DOUBLE, "stance"}, {DOUBLE, "z"},
                                                       {BOOLEAN, "onGround"}, {END}},
    [PACKET_PLAYER_LOOK]                   = (Field[]){{FLOAT, "yaw"}, {FLOAT, "pitch"}, {BOOLEAN, "onGround"}, {END}},
    [PACKET_PLAYER_POSITION_AND_LOOK]      = (Field[]){{DOUBLE, "x"}, {DOUBLE, "y"}, {DOUBLE, "stance"}, {DOUBLE, "z"},
                                                       {FLOAT, "yaw"}, {FLOAT, "pitch"}, {BOOLEAN, "onGround"}, {END}},
    [PACKET_PLAYER_DIGGING]                = (Field[]){{BYTE, "status"}, {INT, "x"}, {BYTE, "y"}, {INT, "z"},
                                                       {BYTE, "face"}, {END}},
    [PACKET_PLAYER_BLOCK_PLACEMENT]        = (Field[]){{INT, "x"}, {BYTE, "y"}, {BYTE, "direction"}, {SLOT, "heldItem"},
                                                       {BYTE, "curPosX"}, {BYTE, "curPosY"}, {BYTE, "curPosZ"}, {END}},
    [PACKET_HELD_ITEM_CHANGE]              = (Field[]){{SHORT, "slotID"}, {END}},
    [PACKET_USE_BED]                       = (Field[]){{INT, "entityID"}, {BYTE, NULL}, {INT, "x"}, {BYTE, "y"}, {INT, "z"},
                                                       {END}},
    [PACKET_ANIMATION]                     = (Field[]){{INT, "entityID"}, {BYTE, "animation"}, {END}},
    [PACKET_ENTITY_ACTION]                 = (Field[]){{INT, "entityID"}, {BYTE, "action"}, {END}},
    [PACKET_SPAWN_NAMED_ENTITY]            = (Field[]){{INT, "entityID"}, {STRING, "playerName"}, {INT, "x"}, {INT, "y"},
                                                       {INT, "z"}, {BYTE, "yaw"}, {BYTE, "pitch"}, {SHORT, "currItem"},
                                                       {METADATA, "metadata"}, {END}},
    [PACKET_COLLECT_ITEM]                  = (Field[]){{INT, "collectedEID"}, {INT, "collectorEID"}, {END}},
    [PACKET_SPAWN_OBJECT]                  = (Field[]){{INT, "entityID"}, {BYTE, "type"}, {INT, "x"}, {INT, "y"},
                                                       {INT, "z"}, {BYTE, "yaw"}, {BYTE, "pitch"},
                                                       {OBJECTDATA, "odjdata"}, {END}},
    [PACKET_SPAWN_MOB]                     = (Field[]){{INT, "entityID"}, {BYTE, "type"}, {INT, "x"}, {INT, "y"},
                                                       {INT, "z"}, {BYTE, "yaw"}, {BYTE, "pitch"}, {BYTE, "headYaw"},
                                                       {SHORT, "velX"}, {SHORT, "velY"}, {SHORT, "velZ"},
                                                       {METADATA, "metadata"}, {END}},
    [PACKET_SPAWN_PAINTING]                = (Field[]){{INT, "entityID"}, {STRING, "title"}, {INT, "cx"},
                                                       {INT, "cy"}, {INT, "cz"}, {INT, "direction"}, {END}},
    [PACKET_SPAWN_EXPERIENCE_ORB]          = (Field[]){{INT, "entityID"}, {INT, "x"}, {INT, "y"}, {INT, "z"},
                                                       {SHORT, "count"}, {END}},
    [PACKET_ENTITY_VELOCITY]               = (Field[]){{INT, "entityID"}, {SHORT, "velX"}, {SHORT, "velY"}, {SHORT, "velZ"},
                                                       {END}},
    [PACKET_DESTROY_ENTITY]                = (Field[]){{INTARRAY, "entityIDs"}, {END}},
    [PACKET_ENTITY]                        = (Field[]){{INT, "entityID"}, {END}},
    [PACKET_RELATIVE_MOVE]                 = (Field[]){{INT, "entityID"}, {BYTE, "dX"}, {BYTE, "dY"}, {BYTE, "dZ"}, {END}},
    [PACKET_ENTITY_LOOK]                   = (Field[]){{INT, "entityID"}, {BYTE, "yaw"}, {BYTE, "pitch"}, {END}},
    [PACKET_ENTITY_LOOK_AND_RELATIVE_MOVE] = (Field[]){{INT, "entityID"}, {BYTE, "dX"}, {BYTE, "dY"}, {BYTE, "dZ"},
                                                       {BYTE, "yaw"}, {BYTE, "pitch"}, {END}},
    [PACKET_ENTITY_TELEPORT]               = (Field[]){{INT, "entityID"}, {INT, "x"}, {INT, "y"}, {INT, "z"},
                                                       {BYTE, "yaw"}, {BYTE, "pitch"}, {END}},
    [PACKET_ENTITY_HEAD_LOOK]              = (Field[]){{INT, "entityID"}, {BYTE, "headYaw"}, {END}},
    [PACKET_ENTITY_STATUS]                 = (Field[]){{INT, "entityID"}, {BYTE, "status"}, {END}},
    [PACKET_ATTACH_ENTITY]                 = (Field[]){{INT, "entityID"}, {INT, "vehicleID"}, {END}},
    [PACKET_ENTITY_METADATA]               = (Field[]){{INT, "entityID"}, {METADATA, "metadata"}, {END}},
    [PACKET_ENTITY_EFFECT]                 = (Field[]){{INT, "entityID"}, {BYTE, "effectID"}, {BYTE, "aplifier"},
                                                       {SHORT, "duration"}, {END}},
    [PACKET_REMOVE_ENTITY_EFFECT]          = (Field[]){{INT, "entityID"}, {BYTE, "effectID"}, {END}},
    [PACKET_SET_EXPERIENCE]                = (Field[]){{FLOAT, "expBar"}, {SHORT, "level"}, {SHORT, "totalExp"}, {END}},
    [PACKET_CHUNK_DATA]                    = (Field[]){{INT, "x"}, {INT, "y"}, {BOOLEAN, "groundUpContinious"},
                                                       {SHORT, "primaryBitMap"}, {SHORT, "addBitMap"},
                                                       {BYTEARRAY4, "compressedData"}, {END}},
    [PACKET_MULTIBLOCK_CHANGE]             = (Field[]){{INT, "x"}, {INT, "y"}, {SHORT, "recordCount"},
                                                       {BYTEARRAY4, "records"}, {END}},
    [PACKET_BLOCK_CHANGE]                  = (Field[]){{INT, "x"}, {BYTE, "y"}, {INT, "z"}, {SHORT, "blockID"},
                                                       {BYTE, "blockMetadata"}, {END}},
    [PACKET_BLOCK_ACTION]                  = (Field[]){{INT, "x"}, {BYTE, "y"}, {INT, "z"}, {BYTE, "byte1"},
                                                       {BYTE, "byte2"}, {SHORT, "blockID"}, {END}},
    [PACKET_BLOCK_BREAK_ANIMATION]         = (Field[]){{INT, "someWeirdEID"}, {INT, "x"}, {INT, "y"}, {INT, "z"},
                                                       {BYTE, "destroyStage"}, {END}},
    [PACKET_MAP_CHUNK_BULK]                = (Field[]){{SHORT, "columnCount"}, {INT, "dataLen"}, {BOOLEAN, "skyLightSent"},
                                                       {BYTEARRAY_NAMED, "data", "dataLen"},
                                                       {STRUCTARRAY, "records", CHUNK_META_INFORMATION_STRUCT, "columnCount"},
                                                       {END}},
    [PACKET_EXPLOSION]                     = (Field[]){{DOUBLE, "x"}, {DOUBLE, "y"}, {DOUBLE, "z"}, {FLOAT, "radius"},
                                                       {INT, "recordsCount"}, {STRUCTARRAY, "records", EXPLOSION_RECORD_STRUCT,
                                                                               "recordsCount"}, {END}},
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
    fprintf(out, "    static const int PACKET_ID = %d;\n", id);
    for (const Field* class_fields = fields[id]; class_fields->type != END; class_fields++) {
        if (class_fields->name == NULL) continue;
        fprintf(out, "    %s %s;\n", fieldtype_to_cstr(class_fields).data, class_fields->name);
    }
}

TEMPStr fields_list_to_cstr(const Field* fields)
{
    TEMPStr str = {0};
    int pos = 0;
    const Field* f = fields;
    for (; f->type != END; f++) {
        if (f->name == NULL) continue;
        pos += sprintf(&str.data[pos],
                        "%s %s", fieldtype_to_cstr(f).data,
                        f->name);
        if (f[1].type != END)
            pos += sprintf(&str.data[pos], ", ");
    }
    return str;
}

void generate_methods_decl(FILE* out, int id)
{
    fprintf(out, "    %s(%s);\n",
            classNames[id], fields_list_to_cstr(fields[id]).data);
    fprintf(out, "    %s(" BUFSTREAM_CLASSNAME "& stream);\n",
            classNames[id]);
    fprintf(out, "    void send(" BUFSTREAM_CLASSNAME "& stream) const override;\n");
}

TEMPStr io_func_prefix_for_type(const Field* f)
{
    TEMPStr str = {0};
    switch (f->type) {
    case BYTE:
    case BOOLEAN:
    case SHORT:
    case FLOAT:
    case INT:
    case DOUBLE:
    case LONG: {
        sprintf(str.data,
                "be<%s>", fieldtype_to_cstr(f).data);
        return str;
    }
    case STRING: {
        sprintf(str.data,
                 "<%s>", fieldtype_to_cstr(f).data);
        return str;
    }
    case INTARRAY:
        return TEMPORATE("V<uint8_t, int>");
    case BYTEARRAY4:
        return TEMPORATE("V<int, uint8_t>");
    case BYTEARRAY:
        return TEMPORATE("V<short, uint8_t>");
    }

    fprintf(stderr, "ERROR: not implemented for %d\n", f->type);
    assert(0);
}

void generate_class_decl(FILE* out, int id)
{
    fprintf(out, "class %s: private Packet {\n", classNames[id]);
    fprintf(out, "public:\n");
    generate_fields(out, id);
    generate_methods_decl(out, id);
    fprintf(out, "};\n\n");
}

bool is_class_in_fields(const Field* f)
{
    for (; f->type != END; ++f) {
        if (IS_TYPE_CLASS(f->type) && !IS_TYPE_ARRAY(f->type)) return true;
    }

    return false;
}

TEMPStr owned_classes_construct_by_bufstr(const Field* fields)
{
    TEMPStr str = {0};
    int pos = 0;

    if (is_class_in_fields(fields))
        pos += sprintf(&str.data[pos], ":\n    ");
    else return str;
    for (const Field* f = fields; f->type != END; ++f) {
        if (!IS_TYPE_CLASS(f->type) || IS_TYPE_ARRAY(f->type) || f->name == NULL)
            continue;
        pos += sprintf(&str.data[pos], "%s(stream)", f->name);
        if (is_class_in_fields(&f[1]))
            pos += sprintf(&str.data[pos], ",\n    ");
    }

    return str;
}

/* Standard methods as constructors (BufStream&; [fields]) and send method */ 
void generate_methods_defs_for_class(FILE* out, const char* namespace,
                                     const char* classname, int packetid, const Field* fields)
{
    fprintf(out, "%s::%s::%s(%s)\n{\n", namespace, classname, classname,
            fields_list_to_cstr(fields).data);
    for (const Field* f = fields; f->type != END; ++f) {
        if (f->name == NULL) continue;
        fprintf(out, "    this->%s = %s;\n", f->name, f->name);
    }
    fprintf(out, "}\n\n");

    fprintf(out, "%s::%s::%s(" BUFSTREAM_CLASSNAME "& stream)%s\n{\n", namespace, classname, classname,
            owned_classes_construct_by_bufstr(fields).data);
    for (const Field* f = fields; f->type != END; ++f) {
        if (f->name == NULL)
            fprintf(out, "    stream.read%s();\n", io_func_prefix_for_type(f).data);
        else if (IS_TYPE_CLASS(f->type) && !IS_TYPE_ARRAY(f->type))
            continue;
        else if (f->type == BYTEARRAY_NAMED)
            fprintf(out, "    this->%s.resize(%s);\n"
                    "    stream.read(this->%s.data(), %s);\n",
                    f->name, f->argName, f->name,
                    f->argName);
        else if (f->type == STRUCTARRAY)
            fprintf(out, "    this->%s.resize(this->%s);\n"
                    "    for (auto i = 0; i < this->%s; ++i)\n"
                    "        this->%s[i].get(stream);\n",
                    f->name, f->argName2, f->argName2, f->name);
        else
            fprintf(out, "    this->%s = stream.read%s();\n", f->name, io_func_prefix_for_type(f).data);
    }
    fprintf(out, "}\n\n");

    fprintf(out, "void %s::%s::send(" BUFSTREAM_CLASSNAME "& stream) const\n{\n",
            namespace, classname);
    if (packetid >= 0) {
        fprintf(out, "    stream.write<uint8_t>(this->PACKET_ID);\n");
    }
    for (const Field* f = fields; f->type != END; ++f) {
        if (f->name == NULL)
            fprintf(out, "    stream.write%s(0);\n", io_func_prefix_for_type(f).data);
        else if (f->type == SLOT || f->type == METADATA || f->type == OBJECTDATA)
            fprintf(out, "    this->%s.send(stream);\n", f->name);
        else if (f->type == BYTEARRAY_NAMED)
            fprintf(out, "    stream.write(this->%s.data(), this->%s.size());\n",
                    f->name, f->name);            
        else if (f->type == STRUCTARRAY)
            fprintf(out, "    for (auto& n: this->%s)\n"
                    "        n.send(stream);\n", f->name);
        else
            fprintf(out, "    stream.write%s(this->%s);\n", io_func_prefix_for_type(f).data, f->name);
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

    fprintf(out, GENERATED_MARK "\n\n", progname);
    fprintf(out, "#ifndef PACKETS_HPP_\n");
    fprintf(out, "#define PACKETS_HPP_\n\n");
    fprintf(out, "#include <cstdlib>\n");
    fprintf(out, "#include <cstdint>\n");
    fprintf(out, "#include <string>\n");
    fprintf(out, "#include <vector>\n\n");
    fprintf(out, "#include <DataClasses.hpp>\n");
    fprintf(out, "#include <BufStream.hpp>\n\n");
    fprintf(out, "namespace Packets {\n");

    fprintf(out, "struct " CHUNK_META_INFORMATION_STRUCT " {\n"
            "    int chunkX;\n"
            "    int chunkZ;\n"
            "    unsigned short primBitMap;\n"
            "    unsigned short addBitMap;\n"
            "    void get(" BUFSTREAM_CLASSNAME "& stream);\n"
            "    void send(" BUFSTREAM_CLASSNAME "& stream) const;\n"
            "};\n\n");

    fprintf(out, "struct " EXPLOSION_RECORD_STRUCT " {\n"
            "    int8_t x, y, z;\n"
            "    void get(" BUFSTREAM_CLASSNAME "& stream);\n"
            "    void send(" BUFSTREAM_CLASSNAME "& stream) const;\n"
            "};\n\n");
    
    fprintf(out,
            "class Packet {\n"
            "public:\n"
            "    virtual void send(" BUFSTREAM_CLASSNAME "& stream) const = 0;\n"
            "};\n");
    
    for (size_t i = 0; i < ARRAY_LEN(packetIds); ++i) {
        generate_class_decl(out, packetIds[i]);
    }
    fprintf(out, "}\n\n");
    fprintf(out, "#endif // PACKETS_HPP_\n");
    if (decloutput) fclose(out);
    if (defoutput) out = fopen(defoutput, "w");

    fprintf(out, GENERATED_MARK "\n\n", progname);
    if (decloutput) {
        const char* fname = decloutput + strlen(decloutput);
        while (fname > decloutput && fname[-1] != '/') fname--;
        fprintf(out, "#include <%s>\n\n", fname);
    }
    
    for (size_t i = 0; i < ARRAY_LEN(packetIds); ++i) {
        generate_methods_defs_for_class
            (out, "Packets", classNames[packetIds[i]],
             packetIds[i], fields[packetIds[i]]);
    }
    
    fclose(out);
    return 0;
}
