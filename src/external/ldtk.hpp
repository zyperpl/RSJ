#pragma once

#include <nlohmann/json.hpp>

#ifndef NLOHMANN_OPT_HELPER
#define NLOHMANN_OPT_HELPER
namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::shared_ptr<T>> {
        static void to_json(json & j, const std::shared_ptr<T> & opt) {
            if (!opt) j = nullptr; else j = *opt;
        }

        static std::shared_ptr<T> from_json(const json & j) {
            if (j.is_null()) return std::unique_ptr<T>(); else return std::unique_ptr<T>(new T(j.get<T>()));
        }
    };
}
#endif

namespace ldtk {
    using nlohmann::json;

    inline json get_untyped(const json & j, const char * property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<json>();
        }
        return json();
    }

    inline json get_untyped(const json & j, std::string property) {
        return get_untyped(j, property.data());
    }

    template <typename T>
    inline std::shared_ptr<T> get_optional(const json & j, const char * property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<std::shared_ptr<T>>();
        }
        return std::shared_ptr<T>();
    }

    template <typename T>
    inline std::shared_ptr<T> get_optional(const json & j, std::string property) {
        return get_optional<T>(j, property.data());
    }

    /**
     * Possible values: `Manual`, `AfterLoad`, `BeforeSave`, `AfterSave`
     */
    enum class When : int { AFTER_LOAD, AFTER_SAVE, BEFORE_SAVE, MANUAL };

    struct LdtkCustomCommand {
        std::string command;
        /**
         * Possible values: `Manual`, `AfterLoad`, `BeforeSave`, `AfterSave`
         */
        When when;
    };

    /**
     * This object represents a custom sub rectangle in a Tileset image.
     */
    struct TilesetRectangle {
        /**
         * Height in pixels
         */
        int64_t h;
        /**
         * UID of the tileset
         */
        int64_t tileset_uid;
        /**
         * Width in pixels
         */
        int64_t w;
        /**
         * X pixels coordinate of the top-left corner in the Tileset image
         */
        int64_t x;
        /**
         * Y pixels coordinate of the top-left corner in the Tileset image
         */
        int64_t y;
    };

    /**
     * An enum describing how the the Entity tile is rendered inside the Entity bounds. Possible
     * values: `Cover`, `FitInside`, `Repeat`, `Stretch`, `FullSizeCropped`,
     * `FullSizeUncropped`, `NineSlice`
     */
    enum class TileRenderMode : int { COVER, FIT_INSIDE, FULL_SIZE_CROPPED, FULL_SIZE_UNCROPPED, NINE_SLICE, REPEAT, STRETCH };

    struct EntityDefinition {
        /**
         * Base entity color
         */
        std::string color;
        /**
         * Pixel height
         */
        int64_t height;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * An array of 4 dimensions for the up/right/down/left borders (in this order) when using
         * 9-slice mode for `tileRenderMode`.<br/>  If the tileRenderMode is not NineSlice, then
         * this array is empty.<br/>  See: https://en.wikipedia.org/wiki/9-slice_scaling
         */
        std::vector<int64_t> nine_slice_borders;
        /**
         * Pivot X coordinate (from 0 to 1.0)
         */
        double pivot_x;
        /**
         * Pivot Y coordinate (from 0 to 1.0)
         */
        double pivot_y;
        /**
         * An object representing a rectangle from an existing Tileset
         */
        std::shared_ptr<TilesetRectangle> tile_rect;
        /**
         * An enum describing how the the Entity tile is rendered inside the Entity bounds. Possible
         * values: `Cover`, `FitInside`, `Repeat`, `Stretch`, `FullSizeCropped`,
         * `FullSizeUncropped`, `NineSlice`
         */
        TileRenderMode tile_render_mode;
        /**
         * Tileset ID used for optional tile display
         */
        std::shared_ptr<int64_t> tileset_id;
        /**
         * Unique Int identifier
         */
        int64_t uid;
        /**
         * Pixel width
         */
        int64_t width;
    };

    struct EnumValueDefinition {
        /**
         * An array of 4 Int values that refers to the tile in the tileset image: `[ x, y, width,
         * height ]`
         */
        std::shared_ptr<std::vector<int64_t>> tile_src_rect;
        /**
         * Optional color
         */
        int64_t color;
        /**
         * Enum value
         */
        std::string id;
        /**
         * The optional ID of the tile
         */
        std::shared_ptr<int64_t> tile_id;
    };

    struct EnumDefinition {
        /**
         * Relative path to the external file providing this Enum
         */
        std::shared_ptr<std::string> external_rel_path;
        /**
         * Tileset UID if provided
         */
        std::shared_ptr<int64_t> icon_tileset_uid;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * An array of user-defined tags to organize the Enums
         */
        std::vector<std::string> tags;
        /**
         * Unique Int identifier
         */
        int64_t uid;
        /**
         * All possible enum values, with their optional Tile infos.
         */
        std::vector<EnumValueDefinition> values;
    };

    /**
     * IntGrid value definition
     */
    struct IntGridValueDefinition {
        std::string color;
        /**
         * User defined unique identifier
         */
        std::shared_ptr<std::string> identifier;
        /**
         * The IntGrid value itself
         */
        int64_t value;
    };

    struct LayerDefinition {
        /**
         * Type of the layer (*IntGrid, Entities, Tiles or AutoLayer*)
         */
        std::string type;
        std::shared_ptr<int64_t> auto_source_layer_def_uid;
        /**
         * Opacity of the layer (0 to 1.0)
         */
        double display_opacity;
        /**
         * Width and height of the grid in pixels
         */
        int64_t grid_size;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * An array that defines extra optional info for each IntGrid value.<br/>  WARNING: the
         * array order is not related to actual IntGrid values! As user can re-order IntGrid values
         * freely, you may value "2" before value "1" in this array.
         */
        std::vector<IntGridValueDefinition> int_grid_values;
        /**
         * Parallax horizontal factor (from -1 to 1, defaults to 0) which affects the scrolling
         * speed of this layer, creating a fake 3D (parallax) effect.
         */
        double parallax_factor_x;
        /**
         * Parallax vertical factor (from -1 to 1, defaults to 0) which affects the scrolling speed
         * of this layer, creating a fake 3D (parallax) effect.
         */
        double parallax_factor_y;
        /**
         * If true (default), a layer with a parallax factor will also be scaled up/down accordingly.
         */
        bool parallax_scaling;
        /**
         * X offset of the layer, in pixels (IMPORTANT: this should be added to the `LayerInstance`
         * optional offset)
         */
        int64_t px_offset_x;
        /**
         * Y offset of the layer, in pixels (IMPORTANT: this should be added to the `LayerInstance`
         * optional offset)
         */
        int64_t px_offset_y;
        /**
         * Reference to the default Tileset UID being used by this layer definition.<br/>
         * **WARNING**: some layer *instances* might use a different tileset. So most of the time,
         * you should probably use the `__tilesetDefUid` value found in layer instances.<br/>  Note:
         * since version 1.0.0, the old `autoTilesetDefUid` was removed and merged into this value.
         */
        std::shared_ptr<int64_t> tileset_def_uid;
        /**
         * Unique Int identifier
         */
        int64_t uid;
    };

    /**
     * This section is mostly only intended for the LDtk editor app itself. You can safely
     * ignore it.
     */
    struct FieldDefinition {
    };

    /**
     * In a tileset definition, user defined meta-data of a tile.
     */
    struct TileCustomMetadata {
        std::string data;
        int64_t tile_id;
    };

    enum class EmbedAtlas : int { LDTK_ICONS };

    /**
     * In a tileset definition, enum based tag infos
     */
    struct EnumTagValue {
        std::string enum_value_id;
        std::vector<int64_t> tile_ids;
    };

    /**
     * The `Tileset` definition is the most important part among project definitions. It
     * contains some extra informations about each integrated tileset. If you only had to parse
     * one definition section, that would be the one.
     */
    struct TilesetDefinition {
        /**
         * Grid-based height
         */
        int64_t c_hei;
        /**
         * Grid-based width
         */
        int64_t c_wid;
        /**
         * An array of custom tile metadata
         */
        std::vector<TileCustomMetadata> custom_data;
        /**
         * If this value is set, then it means that this atlas uses an internal LDtk atlas image
         * instead of a loaded one. Possible values: &lt;`null`&gt;, `LdtkIcons`, `null`
         */
        std::shared_ptr<EmbedAtlas> embed_atlas;
        /**
         * Tileset tags using Enum values specified by `tagsSourceEnumId`. This array contains 1
         * element per Enum value, which contains an array of all Tile IDs that are tagged with it.
         */
        std::vector<EnumTagValue> enum_tags;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * Distance in pixels from image borders
         */
        int64_t padding;
        /**
         * Image height in pixels
         */
        int64_t px_hei;
        /**
         * Image width in pixels
         */
        int64_t px_wid;
        /**
         * Path to the source file, relative to the current project JSON file<br/>  It can be null
         * if no image was provided, or when using an embed atlas.
         */
        std::shared_ptr<std::string> rel_path;
        /**
         * Space in pixels between all tiles
         */
        int64_t spacing;
        /**
         * An array of user-defined tags to organize the Tilesets
         */
        std::vector<std::string> tags;
        /**
         * Optional Enum definition UID used for this tileset meta-data
         */
        std::shared_ptr<int64_t> tags_source_enum_uid;
        int64_t tile_grid_size;
        /**
         * Unique Intidentifier
         */
        int64_t uid;
    };

    /**
     * If you're writing your own LDtk importer, you should probably just ignore *most* stuff in
     * the `defs` section, as it contains data that are mostly important to the editor. To keep
     * you away from the `defs` section and avoid some unnecessary JSON parsing, important data
     * from definitions is often duplicated in fields prefixed with a double underscore (eg.
     * `__identifier` or `__type`).  The 2 only definition types you might need here are
     * **Tilesets** and **Enums**.
     *
     * A structure containing all the definitions of this project
     */
    struct Definitions {
        /**
         * All entities definitions, including their custom fields
         */
        std::vector<EntityDefinition> entities;
        /**
         * All internal enums
         */
        std::vector<EnumDefinition> enums;
        /**
         * Note: external enums are exactly the same as `enums`, except they have a `relPath` to
         * point to an external source file.
         */
        std::vector<EnumDefinition> external_enums;
        /**
         * All layer definitions
         */
        std::vector<LayerDefinition> layers;
        /**
         * All custom fields available to all levels.
         */
        std::vector<FieldDefinition> level_fields;
        /**
         * All tilesets
         */
        std::vector<TilesetDefinition> tilesets;
    };

    /**
     * This complex section isn't meant to be used by game devs at all, as these rules are
     * completely resolved internally by the editor before any saving. You should just ignore
     * this part.
     */
    struct AutoLayerRuleDefinition {
    };

    struct AutoLayerRuleGroup {
        bool active;
        bool is_optional;
        std::string name;
        std::vector<AutoLayerRuleDefinition> rules;
        int64_t uid;
        bool uses_wizard;
    };

    struct FieldInstance {
        /**
         * Field definition identifier
         */
        std::string identifier;
        /**
         * Optional TilesetRect used to display this field (this can be the field own Tile, or some
         * other Tile guessed from the value, like an Enum).
         */
        std::shared_ptr<TilesetRectangle> tile;
        /**
         * Type of the field, such as `Int`, `Float`, `String`, `Enum(my_enum_name)`, `Bool`,
         * etc.<br/>  NOTE: if you enable the advanced option **Use Multilines type**, you will have
         * "*Multilines*" instead of "*String*" when relevant.
         */
        std::string type;
        /**
         * Actual value of the field instance. The value type varies, depending on `__type`:<br/>
         * - For **classic types** (ie. Integer, Float, Boolean, String, Text and FilePath), you
         * just get the actual value with the expected type.<br/>   - For **Color**, the value is an
         * hexadecimal string using "#rrggbb" format.<br/>   - For **Enum**, the value is a String
         * representing the selected enum value.<br/>   - For **Point**, the value is a
         * [GridPoint](#ldtk-GridPoint) object.<br/>   - For **Tile**, the value is a
         * [TilesetRect](#ldtk-TilesetRect) object.<br/>   - For **EntityRef**, the value is an
         * [EntityReferenceInfos](#ldtk-EntityReferenceInfos) object.<br/><br/>  If the field is an
         * array, then this `__value` will also be a JSON array.
         */
        nlohmann::json value;
        /**
         * Reference of the **Field definition** UID
         */
        int64_t def_uid;
    };

    struct EntityInstance {
        /**
         * Grid-based coordinates (`[x,y]` format)
         */
        std::vector<int64_t> grid;
        /**
         * Entity definition identifier
         */
        std::string identifier;
        /**
         * Pivot coordinates  (`[x,y]` format, values are from 0 to 1) of the Entity
         */
        std::vector<double> pivot;
        /**
         * The entity "smart" color, guessed from either Entity definition, or one its field
         * instances.
         */
        std::string smart_color;
        /**
         * Array of tags defined in this Entity definition
         */
        std::vector<std::string> tags;
        /**
         * Optional TilesetRect used to display this entity (it could either be the default Entity
         * tile, or some tile provided by a field value, like an Enum).
         */
        std::shared_ptr<TilesetRectangle> tile;
        /**
         * Reference of the **Entity definition** UID
         */
        int64_t def_uid;
        /**
         * An array of all custom fields and their values.
         */
        std::vector<FieldInstance> field_instances;
        /**
         * Entity height in pixels. For non-resizable entities, it will be the same as Entity
         * definition.
         */
        int64_t height;
        /**
         * Unique instance identifier
         */
        std::string iid;
        /**
         * Pixel coordinates (`[x,y]` format) in current level coordinate space. Don't forget
         * optional layer offsets, if they exist!
         */
        std::vector<int64_t> px;
        /**
         * Entity width in pixels. For non-resizable entities, it will be the same as Entity
         * definition.
         */
        int64_t width;
    };

    /**
     * This object is used in Field Instances to describe an EntityRef value.
     */
    struct FieldInstanceEntityReference {
        /**
         * IID of the refered EntityInstance
         */
        std::string entity_iid;
        /**
         * IID of the LayerInstance containing the refered EntityInstance
         */
        std::string layer_iid;
        /**
         * IID of the Level containing the refered EntityInstance
         */
        std::string level_iid;
        /**
         * IID of the World containing the refered EntityInstance
         */
        std::string world_iid;
    };

    /**
     * This object is just a grid-based coordinate used in Field values.
     */
    struct FieldInstanceGridPoint {
        /**
         * X grid-based coordinate
         */
        int64_t cx;
        /**
         * Y grid-based coordinate
         */
        int64_t cy;
    };

    /**
     * IntGrid value instance
     */
    struct IntGridValueInstance {
        /**
         * Coordinate ID in the layer grid
         */
        int64_t coord_id;
        /**
         * IntGrid value
         */
        int64_t v;
    };

    /**
     * This structure represents a single tile from a given Tileset.
     */
    struct TileInstance {
        /**
         * "Flip bits", a 2-bits integer to represent the mirror transformations of the tile.<br/>
         * - Bit 0 = X flip<br/>   - Bit 1 = Y flip<br/>   Examples: f=0 (no flip), f=1 (X flip
         * only), f=2 (Y flip only), f=3 (both flips)
         */
        int64_t f;
        /**
         * Pixel coordinates of the tile in the **layer** (`[x,y]` format). Don't forget optional
         * layer offsets, if they exist!
         */
        std::vector<int64_t> px;
        /**
         * Pixel coordinates of the tile in the **tileset** (`[x,y]` format)
         */
        std::vector<int64_t> src;
        /**
         * The *Tile ID* in the corresponding tileset.
         */
        int64_t t;
    };

    struct LayerInstance {
        /**
         * Grid-based height
         */
        int64_t c_hei;
        /**
         * Grid-based width
         */
        int64_t c_wid;
        /**
         * Grid size
         */
        int64_t grid_size;
        /**
         * Layer definition identifier
         */
        std::string identifier;
        /**
         * Layer opacity as Float [0-1]
         */
        double opacity;
        /**
         * Total layer X pixel offset, including both instance and definition offsets.
         */
        int64_t px_total_offset_x;
        /**
         * Total layer Y pixel offset, including both instance and definition offsets.
         */
        int64_t px_total_offset_y;
        /**
         * The definition UID of corresponding Tileset, if any.
         */
        std::shared_ptr<int64_t> tileset_def_uid;
        /**
         * The relative path to corresponding Tileset, if any.
         */
        std::shared_ptr<std::string> tileset_rel_path;
        /**
         * Layer type (possible values: IntGrid, Entities, Tiles or AutoLayer)
         */
        std::string type;
        /**
         * An array containing all tiles generated by Auto-layer rules. The array is already sorted
         * in display order (ie. 1st tile is beneath 2nd, which is beneath 3rd etc.).<br/><br/>
         * Note: if multiple tiles are stacked in the same cell as the result of different rules,
         * all tiles behind opaque ones will be discarded.
         */
        std::vector<TileInstance> auto_layer_tiles;
        std::vector<EntityInstance> entity_instances;
        std::vector<TileInstance> grid_tiles;
        /**
         * Unique layer instance identifier
         */
        std::string iid;
        /**
         * A list of all values in the IntGrid layer, stored in CSV format (Comma Separated
         * Values).<br/>  Order is from left to right, and top to bottom (ie. first row from left to
         * right, followed by second row, etc).<br/>  `0` means "empty cell" and IntGrid values
         * start at 1.<br/>  The array size is `__cWid` x `__cHei` cells.
         */
        std::vector<int64_t> int_grid_csv;
        /**
         * Reference the Layer definition UID
         */
        int64_t layer_def_uid;
        /**
         * Reference to the UID of the level containing this layer instance
         */
        int64_t level_id;
        /**
         * This layer can use another tileset by overriding the tileset UID here.
         */
        std::shared_ptr<int64_t> override_tileset_uid;
        /**
         * X offset in pixels to render this layer, usually 0 (IMPORTANT: this should be added to
         * the `LayerDef` optional offset, see `__pxTotalOffsetX`)
         */
        int64_t px_offset_x;
        /**
         * Y offset in pixels to render this layer, usually 0 (IMPORTANT: this should be added to
         * the `LayerDef` optional offset, see `__pxTotalOffsetY`)
         */
        int64_t px_offset_y;
        /**
         * Layer instance visibility
         */
        bool visible;
    };

    /**
     * Level background image position info
     */
    struct LevelBackgroundPosition {
        /**
         * An array of 4 float values describing the cropped sub-rectangle of the displayed
         * background image. This cropping happens when original is larger than the level bounds.
         * Array format: `[ cropX, cropY, cropWidth, cropHeight ]`
         */
        std::vector<double> crop_rect;
        /**
         * An array containing the `[scaleX,scaleY]` values of the **cropped** background image,
         * depending on `bgPos` option.
         */
        std::vector<double> scale;
        /**
         * An array containing the `[x,y]` pixel coordinates of the top-left corner of the
         * **cropped** background image, depending on `bgPos` option.
         */
        std::vector<int64_t> top_left_px;
    };

    /**
     * Nearby level info
     */
    struct NeighbourLevel {
        /**
         * A single lowercase character tipping on the level location (`n`orth, `s`outh, `w`est,
         * `e`ast).
         */
        std::string dir;
        /**
         * Neighbour Instance Identifier
         */
        std::string level_iid;
    };

    /**
     * This section contains all the level data. It can be found in 2 distinct forms, depending
     * on Project current settings:  - If "*Separate level files*" is **disabled** (default):
     * full level data is *embedded* inside the main Project JSON file, - If "*Separate level
     * files*" is **enabled**: level data is stored in *separate* standalone `.ldtkl` files (one
     * per level). In this case, the main Project JSON file will still contain most level data,
     * except heavy sections, like the `layerInstances` array (which will be null). The
     * `externalRelPath` string points to the `ldtkl` file.  A `ldtkl` file is just a JSON file
     * containing exactly what is described below.
     */
    struct Level {
        /**
         * Background color of the level (same as `bgColor`, except the default value is
         * automatically used here if its value is `null`)
         */
        std::string bg_color;
        /**
         * Position informations of the background image, if there is one.
         */
        std::shared_ptr<LevelBackgroundPosition> bg_pos;
        /**
         * An array listing all other levels touching this one on the world map.<br/>  Only relevant
         * for world layouts where level spatial positioning is manual (ie. GridVania, Free). For
         * Horizontal and Vertical layouts, this array is always empty.
         */
        std::vector<NeighbourLevel> neighbours;
        /**
         * The *optional* relative path to the level background image.
         */
        std::shared_ptr<std::string> bg_rel_path;
        /**
         * This value is not null if the project option "*Save levels separately*" is enabled. In
         * this case, this **relative** path points to the level Json file.
         */
        std::shared_ptr<std::string> external_rel_path;
        /**
         * An array containing this level custom field values.
         */
        std::vector<FieldInstance> field_instances;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * Unique instance identifier
         */
        std::string iid;
        /**
         * An array containing all Layer instances. **IMPORTANT**: if the project option "*Save
         * levels separately*" is enabled, this field will be `null`.<br/>  This array is **sorted
         * in display order**: the 1st layer is the top-most and the last is behind.
         */
        std::shared_ptr<std::vector<LayerInstance>> layer_instances;
        /**
         * Height of the level in pixels
         */
        int64_t px_hei;
        /**
         * Width of the level in pixels
         */
        int64_t px_wid;
        /**
         * Unique Int identifier
         */
        int64_t uid;
        /**
         * Index that represents the "depth" of the level in the world. Default is 0, greater means
         * "above", lower means "below".<br/>  This value is mostly used for display only and is
         * intended to make stacking of levels easier to manage.
         */
        int64_t world_depth;
        /**
         * World X coordinate in pixels.<br/>  Only relevant for world layouts where level spatial
         * positioning is manual (ie. GridVania, Free). For Horizontal and Vertical layouts, the
         * value is always -1 here.
         */
        int64_t world_x;
        /**
         * World Y coordinate in pixels.<br/>  Only relevant for world layouts where level spatial
         * positioning is manual (ie. GridVania, Free). For Horizontal and Vertical layouts, the
         * value is always -1 here.
         */
        int64_t world_y;
    };

    enum class WorldLayout : int { FREE, GRID_VANIA, LINEAR_HORIZONTAL, LINEAR_VERTICAL };

    /**
     * **IMPORTANT**: this type is not used *yet* in current LDtk version. It's only presented
     * here as a preview of a planned feature.  A World contains multiple levels, and it has its
     * own layout settings.
     */
    struct World {
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * Unique instance identifer
         */
        std::string iid;
        /**
         * All levels from this world. The order of this array is only relevant in
         * `LinearHorizontal` and `linearVertical` world layouts (see `worldLayout` value).
         * Otherwise, you should refer to the `worldX`,`worldY` coordinates of each Level.
         */
        std::vector<Level> levels;
        /**
         * Height of the world grid in pixels.
         */
        int64_t world_grid_height;
        /**
         * Width of the world grid in pixels.
         */
        int64_t world_grid_width;
        /**
         * An enum that describes how levels are organized in this project (ie. linearly or in a 2D
         * space). Possible values: `Free`, `GridVania`, `LinearHorizontal`, `LinearVertical`,
         * `null`, `null`
         */
        std::shared_ptr<WorldLayout> world_layout;
    };

    /**
     * This object is not actually used by LDtk. It ONLY exists to force explicit references to
     * all types, to make sure QuickType finds them and integrate all of them. Otherwise,
     * Quicktype will drop types that are not explicitely used.
     */
    struct ForcedRefs {
        std::shared_ptr<AutoLayerRuleGroup> auto_layer_rule_group;
        std::shared_ptr<AutoLayerRuleDefinition> auto_rule_def;
        std::shared_ptr<LdtkCustomCommand> custom_command;
        std::shared_ptr<Definitions> definitions;
        std::shared_ptr<EntityDefinition> entity_def;
        std::shared_ptr<EntityInstance> entity_instance;
        std::shared_ptr<FieldInstanceEntityReference> entity_reference_infos;
        std::shared_ptr<EnumDefinition> enum_def;
        std::shared_ptr<EnumValueDefinition> enum_def_values;
        std::shared_ptr<EnumTagValue> enum_tag_value;
        std::shared_ptr<FieldDefinition> field_def;
        std::shared_ptr<FieldInstance> field_instance;
        std::shared_ptr<FieldInstanceGridPoint> grid_point;
        std::shared_ptr<IntGridValueDefinition> int_grid_value_def;
        std::shared_ptr<IntGridValueInstance> int_grid_value_instance;
        std::shared_ptr<LayerDefinition> layer_def;
        std::shared_ptr<LayerInstance> layer_instance;
        std::shared_ptr<Level> level;
        std::shared_ptr<LevelBackgroundPosition> level_bg_pos_infos;
        std::shared_ptr<NeighbourLevel> neighbour_level;
        std::shared_ptr<TileInstance> tile;
        std::shared_ptr<TileCustomMetadata> tile_custom_metadata;
        std::shared_ptr<TilesetDefinition> tileset_def;
        std::shared_ptr<TilesetRectangle> tileset_rect;
        std::shared_ptr<World> world;
    };

    /**
     * This file is a JSON schema of files created by LDtk level editor (https://ldtk.io).
     *
     * This is the root of any Project JSON file. It contains:  - the project settings, - an
     * array of levels, - a group of definitions (that can probably be safely ignored for most
     * users).
     */
    struct Project {
        /**
         * This object is not actually used by LDtk. It ONLY exists to force explicit references to
         * all types, to make sure QuickType finds them and integrate all of them. Otherwise,
         * Quicktype will drop types that are not explicitely used.
         */
        std::shared_ptr<ForcedRefs> forced_refs;
        /**
         * Project background color
         */
        std::string bg_color;
        /**
         * An array of command lines that can be ran manually by the user
         */
        std::vector<LdtkCustomCommand> custom_commands;
        /**
         * A structure containing all the definitions of this project
         */
        Definitions defs;
        /**
         * If TRUE, one file will be saved for the project (incl. all its definitions) and one file
         * in a sub-folder for each level.
         */
        bool external_levels;
        /**
         * Unique project identifier
         */
        std::string iid;
        /**
         * File format version
         */
        std::string json_version;
        /**
         * All levels. The order of this array is only relevant in `LinearHorizontal` and
         * `linearVertical` world layouts (see `worldLayout` value).<br/>  Otherwise, you should
         * refer to the `worldX`,`worldY` coordinates of each Level.
         */
        std::vector<Level> levels;
        /**
         * **WARNING**: this field will move to the `worlds` array after the "multi-worlds" update.
         * It will then be `null`. You can enable the Multi-worlds advanced project option to enable
         * the change immediately.<br/><br/>  Height of the world grid in pixels.
         */
        std::shared_ptr<int64_t> world_grid_height;
        /**
         * **WARNING**: this field will move to the `worlds` array after the "multi-worlds" update.
         * It will then be `null`. You can enable the Multi-worlds advanced project option to enable
         * the change immediately.<br/><br/>  Width of the world grid in pixels.
         */
        std::shared_ptr<int64_t> world_grid_width;
        /**
         * **WARNING**: this field will move to the `worlds` array after the "multi-worlds" update.
         * It will then be `null`. You can enable the Multi-worlds advanced project option to enable
         * the change immediately.<br/><br/>  An enum that describes how levels are organized in
         * this project (ie. linearly or in a 2D space). Possible values: &lt;`null`&gt;, `Free`,
         * `GridVania`, `LinearHorizontal`, `LinearVertical`, `null`
         */
        std::shared_ptr<WorldLayout> world_layout;
        /**
         * This array is not used yet in current LDtk version (so, for now, it's always
         * empty).<br/><br/>In a later update, it will be possible to have multiple Worlds in a
         * single project, each containing multiple Levels.<br/><br/>What will change when "Multiple
         * worlds" support will be added to LDtk:<br/><br/> - in current version, a LDtk project
         * file can only contain a single world with multiple levels in it. In this case, levels and
         * world layout related settings are stored in the root of the JSON.<br/> - after the
         * "Multiple worlds" update, there will be a `worlds` array in root, each world containing
         * levels and layout settings. Basically, it's pretty much only about moving the `levels`
         * array to the `worlds` array, along with world layout related values (eg. `worldGridWidth`
         * etc).<br/><br/>If you want to start supporting this future update easily, please refer to
         * this documentation: https://github.com/deepnight/ldtk/issues/231
         */
        std::vector<World> worlds;
    };
}

namespace nlohmann {
    void from_json(const json & j, ldtk::LdtkCustomCommand & x);
    void to_json(json & j, const ldtk::LdtkCustomCommand & x);

    void from_json(const json & j, ldtk::TilesetRectangle & x);
    void to_json(json & j, const ldtk::TilesetRectangle & x);

    void from_json(const json & j, ldtk::EntityDefinition & x);
    void to_json(json & j, const ldtk::EntityDefinition & x);

    void from_json(const json & j, ldtk::EnumValueDefinition & x);
    void to_json(json & j, const ldtk::EnumValueDefinition & x);

    void from_json(const json & j, ldtk::EnumDefinition & x);
    void to_json(json & j, const ldtk::EnumDefinition & x);

    void from_json(const json & j, ldtk::IntGridValueDefinition & x);
    void to_json(json & j, const ldtk::IntGridValueDefinition & x);

    void from_json(const json & j, ldtk::LayerDefinition & x);
    void to_json(json & j, const ldtk::LayerDefinition & x);

    void from_json(const json & j, ldtk::FieldDefinition & x);
    void to_json(json & j, const ldtk::FieldDefinition & x);

    void from_json(const json & j, ldtk::TileCustomMetadata & x);
    void to_json(json & j, const ldtk::TileCustomMetadata & x);

    void from_json(const json & j, ldtk::EnumTagValue & x);
    void to_json(json & j, const ldtk::EnumTagValue & x);

    void from_json(const json & j, ldtk::TilesetDefinition & x);
    void to_json(json & j, const ldtk::TilesetDefinition & x);

    void from_json(const json & j, ldtk::Definitions & x);
    void to_json(json & j, const ldtk::Definitions & x);

    void from_json(const json & j, ldtk::AutoLayerRuleDefinition & x);
    void to_json(json & j, const ldtk::AutoLayerRuleDefinition & x);

    void from_json(const json & j, ldtk::AutoLayerRuleGroup & x);
    void to_json(json & j, const ldtk::AutoLayerRuleGroup & x);

    void from_json(const json & j, ldtk::FieldInstance & x);
    void to_json(json & j, const ldtk::FieldInstance & x);

    void from_json(const json & j, ldtk::EntityInstance & x);
    void to_json(json & j, const ldtk::EntityInstance & x);

    void from_json(const json & j, ldtk::FieldInstanceEntityReference & x);
    void to_json(json & j, const ldtk::FieldInstanceEntityReference & x);

    void from_json(const json & j, ldtk::FieldInstanceGridPoint & x);
    void to_json(json & j, const ldtk::FieldInstanceGridPoint & x);

    void from_json(const json & j, ldtk::IntGridValueInstance & x);
    void to_json(json & j, const ldtk::IntGridValueInstance & x);

    void from_json(const json & j, ldtk::TileInstance & x);
    void to_json(json & j, const ldtk::TileInstance & x);

    void from_json(const json & j, ldtk::LayerInstance & x);
    void to_json(json & j, const ldtk::LayerInstance & x);

    void from_json(const json & j, ldtk::LevelBackgroundPosition & x);
    void to_json(json & j, const ldtk::LevelBackgroundPosition & x);

    void from_json(const json & j, ldtk::NeighbourLevel & x);
    void to_json(json & j, const ldtk::NeighbourLevel & x);

    void from_json(const json & j, ldtk::Level & x);
    void to_json(json & j, const ldtk::Level & x);

    void from_json(const json & j, ldtk::World & x);
    void to_json(json & j, const ldtk::World & x);

    void from_json(const json & j, ldtk::ForcedRefs & x);
    void to_json(json & j, const ldtk::ForcedRefs & x);

    void from_json(const json & j, ldtk::Project & x);
    void to_json(json & j, const ldtk::Project & x);

    void from_json(const json & j, ldtk::When & x);
    void to_json(json & j, const ldtk::When & x);

    void from_json(const json & j, ldtk::TileRenderMode & x);
    void to_json(json & j, const ldtk::TileRenderMode & x);

    void from_json(const json & j, ldtk::EmbedAtlas & x);
    void to_json(json & j, const ldtk::EmbedAtlas & x);

    void from_json(const json & j, ldtk::WorldLayout & x);
    void to_json(json & j, const ldtk::WorldLayout & x);

    inline void from_json(const json & j, ldtk::LdtkCustomCommand& x) {
        x.command = j.at("command").get<std::string>();
        x.when = j.at("when").get<ldtk::When>();
    }

    inline void to_json(json & j, const ldtk::LdtkCustomCommand & x) {
        j = json::object();
        j["command"] = x.command;
        j["when"] = x.when;
    }

    inline void from_json(const json & j, ldtk::TilesetRectangle& x) {
        x.h = j.at("h").get<int64_t>();
        x.tileset_uid = j.at("tilesetUid").get<int64_t>();
        x.w = j.at("w").get<int64_t>();
        x.x = j.at("x").get<int64_t>();
        x.y = j.at("y").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::TilesetRectangle & x) {
        j = json::object();
        j["h"] = x.h;
        j["tilesetUid"] = x.tileset_uid;
        j["w"] = x.w;
        j["x"] = x.x;
        j["y"] = x.y;
    }

    inline void from_json(const json & j, ldtk::EntityDefinition& x) {
        x.color = j.at("color").get<std::string>();
        x.height = j.at("height").get<int64_t>();
        x.identifier = j.at("identifier").get<std::string>();
        x.nine_slice_borders = j.at("nineSliceBorders").get<std::vector<int64_t>>();
        x.pivot_x = j.at("pivotX").get<double>();
        x.pivot_y = j.at("pivotY").get<double>();
        x.tile_rect = ldtk::get_optional<ldtk::TilesetRectangle>(j, "tileRect");
        x.tile_render_mode = j.at("tileRenderMode").get<ldtk::TileRenderMode>();
        x.tileset_id = ldtk::get_optional<int64_t>(j, "tilesetId");
        x.uid = j.at("uid").get<int64_t>();
        x.width = j.at("width").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::EntityDefinition & x) {
        j = json::object();
        j["color"] = x.color;
        j["height"] = x.height;
        j["identifier"] = x.identifier;
        j["nineSliceBorders"] = x.nine_slice_borders;
        j["pivotX"] = x.pivot_x;
        j["pivotY"] = x.pivot_y;
        j["tileRect"] = x.tile_rect;
        j["tileRenderMode"] = x.tile_render_mode;
        j["tilesetId"] = x.tileset_id;
        j["uid"] = x.uid;
        j["width"] = x.width;
    }

    inline void from_json(const json & j, ldtk::EnumValueDefinition& x) {
        x.tile_src_rect = ldtk::get_optional<std::vector<int64_t>>(j, "__tileSrcRect");
        x.color = j.at("color").get<int64_t>();
        x.id = j.at("id").get<std::string>();
        x.tile_id = ldtk::get_optional<int64_t>(j, "tileId");
    }

    inline void to_json(json & j, const ldtk::EnumValueDefinition & x) {
        j = json::object();
        j["__tileSrcRect"] = x.tile_src_rect;
        j["color"] = x.color;
        j["id"] = x.id;
        j["tileId"] = x.tile_id;
    }

    inline void from_json(const json & j, ldtk::EnumDefinition& x) {
        x.external_rel_path = ldtk::get_optional<std::string>(j, "externalRelPath");
        x.icon_tileset_uid = ldtk::get_optional<int64_t>(j, "iconTilesetUid");
        x.identifier = j.at("identifier").get<std::string>();
        x.tags = j.at("tags").get<std::vector<std::string>>();
        x.uid = j.at("uid").get<int64_t>();
        x.values = j.at("values").get<std::vector<ldtk::EnumValueDefinition>>();
    }

    inline void to_json(json & j, const ldtk::EnumDefinition & x) {
        j = json::object();
        j["externalRelPath"] = x.external_rel_path;
        j["iconTilesetUid"] = x.icon_tileset_uid;
        j["identifier"] = x.identifier;
        j["tags"] = x.tags;
        j["uid"] = x.uid;
        j["values"] = x.values;
    }

    inline void from_json(const json & j, ldtk::IntGridValueDefinition& x) {
        x.color = j.at("color").get<std::string>();
        x.identifier = ldtk::get_optional<std::string>(j, "identifier");
        x.value = j.at("value").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::IntGridValueDefinition & x) {
        j = json::object();
        j["color"] = x.color;
        j["identifier"] = x.identifier;
        j["value"] = x.value;
    }

    inline void from_json(const json & j, ldtk::LayerDefinition& x) {
        x.type = j.at("__type").get<std::string>();
        x.auto_source_layer_def_uid = ldtk::get_optional<int64_t>(j, "autoSourceLayerDefUid");
        x.display_opacity = j.at("displayOpacity").get<double>();
        x.grid_size = j.at("gridSize").get<int64_t>();
        x.identifier = j.at("identifier").get<std::string>();
        x.int_grid_values = j.at("intGridValues").get<std::vector<ldtk::IntGridValueDefinition>>();
        x.parallax_factor_x = j.at("parallaxFactorX").get<double>();
        x.parallax_factor_y = j.at("parallaxFactorY").get<double>();
        x.parallax_scaling = j.at("parallaxScaling").get<bool>();
        x.px_offset_x = j.at("pxOffsetX").get<int64_t>();
        x.px_offset_y = j.at("pxOffsetY").get<int64_t>();
        x.tileset_def_uid = ldtk::get_optional<int64_t>(j, "tilesetDefUid");
        x.uid = j.at("uid").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::LayerDefinition & x) {
        j = json::object();
        j["__type"] = x.type;
        j["autoSourceLayerDefUid"] = x.auto_source_layer_def_uid;
        j["displayOpacity"] = x.display_opacity;
        j["gridSize"] = x.grid_size;
        j["identifier"] = x.identifier;
        j["intGridValues"] = x.int_grid_values;
        j["parallaxFactorX"] = x.parallax_factor_x;
        j["parallaxFactorY"] = x.parallax_factor_y;
        j["parallaxScaling"] = x.parallax_scaling;
        j["pxOffsetX"] = x.px_offset_x;
        j["pxOffsetY"] = x.px_offset_y;
        j["tilesetDefUid"] = x.tileset_def_uid;
        j["uid"] = x.uid;
    }

    inline void from_json(const json &, ldtk::FieldDefinition&) {
    }

    inline void to_json(json & j, const ldtk::FieldDefinition &) {
        j = json::object();
    }

    inline void from_json(const json & j, ldtk::TileCustomMetadata& x) {
        x.data = j.at("data").get<std::string>();
        x.tile_id = j.at("tileId").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::TileCustomMetadata & x) {
        j = json::object();
        j["data"] = x.data;
        j["tileId"] = x.tile_id;
    }

    inline void from_json(const json & j, ldtk::EnumTagValue& x) {
        x.enum_value_id = j.at("enumValueId").get<std::string>();
        x.tile_ids = j.at("tileIds").get<std::vector<int64_t>>();
    }

    inline void to_json(json & j, const ldtk::EnumTagValue & x) {
        j = json::object();
        j["enumValueId"] = x.enum_value_id;
        j["tileIds"] = x.tile_ids;
    }

    inline void from_json(const json & j, ldtk::TilesetDefinition& x) {
        x.c_hei = j.at("__cHei").get<int64_t>();
        x.c_wid = j.at("__cWid").get<int64_t>();
        x.custom_data = j.at("customData").get<std::vector<ldtk::TileCustomMetadata>>();
        x.embed_atlas = ldtk::get_optional<ldtk::EmbedAtlas>(j, "embedAtlas");
        x.enum_tags = j.at("enumTags").get<std::vector<ldtk::EnumTagValue>>();
        x.identifier = j.at("identifier").get<std::string>();
        x.padding = j.at("padding").get<int64_t>();
        x.px_hei = j.at("pxHei").get<int64_t>();
        x.px_wid = j.at("pxWid").get<int64_t>();
        x.rel_path = ldtk::get_optional<std::string>(j, "relPath");
        x.spacing = j.at("spacing").get<int64_t>();
        x.tags = j.at("tags").get<std::vector<std::string>>();
        x.tags_source_enum_uid = ldtk::get_optional<int64_t>(j, "tagsSourceEnumUid");
        x.tile_grid_size = j.at("tileGridSize").get<int64_t>();
        x.uid = j.at("uid").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::TilesetDefinition & x) {
        j = json::object();
        j["__cHei"] = x.c_hei;
        j["__cWid"] = x.c_wid;
        j["customData"] = x.custom_data;
        j["embedAtlas"] = x.embed_atlas;
        j["enumTags"] = x.enum_tags;
        j["identifier"] = x.identifier;
        j["padding"] = x.padding;
        j["pxHei"] = x.px_hei;
        j["pxWid"] = x.px_wid;
        j["relPath"] = x.rel_path;
        j["spacing"] = x.spacing;
        j["tags"] = x.tags;
        j["tagsSourceEnumUid"] = x.tags_source_enum_uid;
        j["tileGridSize"] = x.tile_grid_size;
        j["uid"] = x.uid;
    }

    inline void from_json(const json & j, ldtk::Definitions& x) {
        x.entities = j.at("entities").get<std::vector<ldtk::EntityDefinition>>();
        x.enums = j.at("enums").get<std::vector<ldtk::EnumDefinition>>();
        x.external_enums = j.at("externalEnums").get<std::vector<ldtk::EnumDefinition>>();
        x.layers = j.at("layers").get<std::vector<ldtk::LayerDefinition>>();
        x.level_fields = j.at("levelFields").get<std::vector<ldtk::FieldDefinition>>();
        x.tilesets = j.at("tilesets").get<std::vector<ldtk::TilesetDefinition>>();
    }

    inline void to_json(json & j, const ldtk::Definitions & x) {
        j = json::object();
        j["entities"] = x.entities;
        j["enums"] = x.enums;
        j["externalEnums"] = x.external_enums;
        j["layers"] = x.layers;
        j["levelFields"] = x.level_fields;
        j["tilesets"] = x.tilesets;
    }

    inline void from_json(const json &, ldtk::AutoLayerRuleDefinition&) {
    }

    inline void to_json(json & j, const ldtk::AutoLayerRuleDefinition &) {
        j = json::object();
    }

    inline void from_json(const json & j, ldtk::AutoLayerRuleGroup& x) {
        x.active = j.at("active").get<bool>();
        x.is_optional = j.at("isOptional").get<bool>();
        x.name = j.at("name").get<std::string>();
        x.rules = j.at("rules").get<std::vector<ldtk::AutoLayerRuleDefinition>>();
        x.uid = j.at("uid").get<int64_t>();
        x.uses_wizard = j.at("usesWizard").get<bool>();
    }

    inline void to_json(json & j, const ldtk::AutoLayerRuleGroup & x) {
        j = json::object();
        j["active"] = x.active;
        j["isOptional"] = x.is_optional;
        j["name"] = x.name;
        j["rules"] = x.rules;
        j["uid"] = x.uid;
        j["usesWizard"] = x.uses_wizard;
    }

    inline void from_json(const json & j, ldtk::FieldInstance& x) {
        x.identifier = j.at("__identifier").get<std::string>();
        x.tile = ldtk::get_optional<ldtk::TilesetRectangle>(j, "__tile");
        x.type = j.at("__type").get<std::string>();
        x.value = ldtk::get_untyped(j, "__value");
        x.def_uid = j.at("defUid").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::FieldInstance & x) {
        j = json::object();
        j["__identifier"] = x.identifier;
        j["__tile"] = x.tile;
        j["__type"] = x.type;
        j["__value"] = x.value;
        j["defUid"] = x.def_uid;
    }

    inline void from_json(const json & j, ldtk::EntityInstance& x) {
        x.grid = j.at("__grid").get<std::vector<int64_t>>();
        x.identifier = j.at("__identifier").get<std::string>();
        x.pivot = j.at("__pivot").get<std::vector<double>>();
        x.smart_color = j.at("__smartColor").get<std::string>();
        x.tags = j.at("__tags").get<std::vector<std::string>>();
        x.tile = ldtk::get_optional<ldtk::TilesetRectangle>(j, "__tile");
        x.def_uid = j.at("defUid").get<int64_t>();
        x.field_instances = j.at("fieldInstances").get<std::vector<ldtk::FieldInstance>>();
        x.height = j.at("height").get<int64_t>();
        x.iid = j.at("iid").get<std::string>();
        x.px = j.at("px").get<std::vector<int64_t>>();
        x.width = j.at("width").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::EntityInstance & x) {
        j = json::object();
        j["__grid"] = x.grid;
        j["__identifier"] = x.identifier;
        j["__pivot"] = x.pivot;
        j["__smartColor"] = x.smart_color;
        j["__tags"] = x.tags;
        j["__tile"] = x.tile;
        j["defUid"] = x.def_uid;
        j["fieldInstances"] = x.field_instances;
        j["height"] = x.height;
        j["iid"] = x.iid;
        j["px"] = x.px;
        j["width"] = x.width;
    }

    inline void from_json(const json & j, ldtk::FieldInstanceEntityReference& x) {
        x.entity_iid = j.at("entityIid").get<std::string>();
        x.layer_iid = j.at("layerIid").get<std::string>();
        x.level_iid = j.at("levelIid").get<std::string>();
        x.world_iid = j.at("worldIid").get<std::string>();
    }

    inline void to_json(json & j, const ldtk::FieldInstanceEntityReference & x) {
        j = json::object();
        j["entityIid"] = x.entity_iid;
        j["layerIid"] = x.layer_iid;
        j["levelIid"] = x.level_iid;
        j["worldIid"] = x.world_iid;
    }

    inline void from_json(const json & j, ldtk::FieldInstanceGridPoint& x) {
        x.cx = j.at("cx").get<int64_t>();
        x.cy = j.at("cy").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::FieldInstanceGridPoint & x) {
        j = json::object();
        j["cx"] = x.cx;
        j["cy"] = x.cy;
    }

    inline void from_json(const json & j, ldtk::IntGridValueInstance& x) {
        x.coord_id = j.at("coordId").get<int64_t>();
        x.v = j.at("v").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::IntGridValueInstance & x) {
        j = json::object();
        j["coordId"] = x.coord_id;
        j["v"] = x.v;
    }

    inline void from_json(const json & j, ldtk::TileInstance& x) {
        x.f = j.at("f").get<int64_t>();
        x.px = j.at("px").get<std::vector<int64_t>>();
        x.src = j.at("src").get<std::vector<int64_t>>();
        x.t = j.at("t").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::TileInstance & x) {
        j = json::object();
        j["f"] = x.f;
        j["px"] = x.px;
        j["src"] = x.src;
        j["t"] = x.t;
    }

    inline void from_json(const json & j, ldtk::LayerInstance& x) {
        x.c_hei = j.at("__cHei").get<int64_t>();
        x.c_wid = j.at("__cWid").get<int64_t>();
        x.grid_size = j.at("__gridSize").get<int64_t>();
        x.identifier = j.at("__identifier").get<std::string>();
        x.opacity = j.at("__opacity").get<double>();
        x.px_total_offset_x = j.at("__pxTotalOffsetX").get<int64_t>();
        x.px_total_offset_y = j.at("__pxTotalOffsetY").get<int64_t>();
        x.tileset_def_uid = ldtk::get_optional<int64_t>(j, "__tilesetDefUid");
        x.tileset_rel_path = ldtk::get_optional<std::string>(j, "__tilesetRelPath");
        x.type = j.at("__type").get<std::string>();
        x.auto_layer_tiles = j.at("autoLayerTiles").get<std::vector<ldtk::TileInstance>>();
        x.entity_instances = j.at("entityInstances").get<std::vector<ldtk::EntityInstance>>();
        x.grid_tiles = j.at("gridTiles").get<std::vector<ldtk::TileInstance>>();
        x.iid = j.at("iid").get<std::string>();
        x.int_grid_csv = j.at("intGridCsv").get<std::vector<int64_t>>();
        x.layer_def_uid = j.at("layerDefUid").get<int64_t>();
        x.level_id = j.at("levelId").get<int64_t>();
        x.override_tileset_uid = ldtk::get_optional<int64_t>(j, "overrideTilesetUid");
        x.px_offset_x = j.at("pxOffsetX").get<int64_t>();
        x.px_offset_y = j.at("pxOffsetY").get<int64_t>();
        x.visible = j.at("visible").get<bool>();
    }

    inline void to_json(json & j, const ldtk::LayerInstance & x) {
        j = json::object();
        j["__cHei"] = x.c_hei;
        j["__cWid"] = x.c_wid;
        j["__gridSize"] = x.grid_size;
        j["__identifier"] = x.identifier;
        j["__opacity"] = x.opacity;
        j["__pxTotalOffsetX"] = x.px_total_offset_x;
        j["__pxTotalOffsetY"] = x.px_total_offset_y;
        j["__tilesetDefUid"] = x.tileset_def_uid;
        j["__tilesetRelPath"] = x.tileset_rel_path;
        j["__type"] = x.type;
        j["autoLayerTiles"] = x.auto_layer_tiles;
        j["entityInstances"] = x.entity_instances;
        j["gridTiles"] = x.grid_tiles;
        j["iid"] = x.iid;
        j["intGridCsv"] = x.int_grid_csv;
        j["layerDefUid"] = x.layer_def_uid;
        j["levelId"] = x.level_id;
        j["overrideTilesetUid"] = x.override_tileset_uid;
        j["pxOffsetX"] = x.px_offset_x;
        j["pxOffsetY"] = x.px_offset_y;
        j["visible"] = x.visible;
    }

    inline void from_json(const json & j, ldtk::LevelBackgroundPosition& x) {
        x.crop_rect = j.at("cropRect").get<std::vector<double>>();
        x.scale = j.at("scale").get<std::vector<double>>();
        x.top_left_px = j.at("topLeftPx").get<std::vector<int64_t>>();
    }

    inline void to_json(json & j, const ldtk::LevelBackgroundPosition & x) {
        j = json::object();
        j["cropRect"] = x.crop_rect;
        j["scale"] = x.scale;
        j["topLeftPx"] = x.top_left_px;
    }

    inline void from_json(const json & j, ldtk::NeighbourLevel& x) {
        x.dir = j.at("dir").get<std::string>();
        x.level_iid = j.at("levelIid").get<std::string>();
    }

    inline void to_json(json & j, const ldtk::NeighbourLevel & x) {
        j = json::object();
        j["dir"] = x.dir;
        j["levelIid"] = x.level_iid;
    }

    inline void from_json(const json & j, ldtk::Level& x) {
        x.bg_color = j.at("__bgColor").get<std::string>();
        x.bg_pos = ldtk::get_optional<ldtk::LevelBackgroundPosition>(j, "__bgPos");
        x.neighbours = j.at("__neighbours").get<std::vector<ldtk::NeighbourLevel>>();
        x.bg_rel_path = ldtk::get_optional<std::string>(j, "bgRelPath");
        x.external_rel_path = ldtk::get_optional<std::string>(j, "externalRelPath");
        x.field_instances = j.at("fieldInstances").get<std::vector<ldtk::FieldInstance>>();
        x.identifier = j.at("identifier").get<std::string>();
        x.iid = j.at("iid").get<std::string>();
        x.layer_instances = ldtk::get_optional<std::vector<ldtk::LayerInstance>>(j, "layerInstances");
        x.px_hei = j.at("pxHei").get<int64_t>();
        x.px_wid = j.at("pxWid").get<int64_t>();
        x.uid = j.at("uid").get<int64_t>();
        x.world_depth = j.at("worldDepth").get<int64_t>();
        x.world_x = j.at("worldX").get<int64_t>();
        x.world_y = j.at("worldY").get<int64_t>();
    }

    inline void to_json(json & j, const ldtk::Level & x) {
        j = json::object();
        j["__bgColor"] = x.bg_color;
        j["__bgPos"] = x.bg_pos;
        j["__neighbours"] = x.neighbours;
        j["bgRelPath"] = x.bg_rel_path;
        j["externalRelPath"] = x.external_rel_path;
        j["fieldInstances"] = x.field_instances;
        j["identifier"] = x.identifier;
        j["iid"] = x.iid;
        j["layerInstances"] = x.layer_instances;
        j["pxHei"] = x.px_hei;
        j["pxWid"] = x.px_wid;
        j["uid"] = x.uid;
        j["worldDepth"] = x.world_depth;
        j["worldX"] = x.world_x;
        j["worldY"] = x.world_y;
    }

    inline void from_json(const json & j, ldtk::World& x) {
        x.identifier = j.at("identifier").get<std::string>();
        x.iid = j.at("iid").get<std::string>();
        x.levels = j.at("levels").get<std::vector<ldtk::Level>>();
        x.world_grid_height = j.at("worldGridHeight").get<int64_t>();
        x.world_grid_width = j.at("worldGridWidth").get<int64_t>();
        x.world_layout = ldtk::get_optional<ldtk::WorldLayout>(j, "worldLayout");
    }

    inline void to_json(json & j, const ldtk::World & x) {
        j = json::object();
        j["identifier"] = x.identifier;
        j["iid"] = x.iid;
        j["levels"] = x.levels;
        j["worldGridHeight"] = x.world_grid_height;
        j["worldGridWidth"] = x.world_grid_width;
        j["worldLayout"] = x.world_layout;
    }

    inline void from_json(const json & j, ldtk::ForcedRefs& x) {
        x.auto_layer_rule_group = ldtk::get_optional<ldtk::AutoLayerRuleGroup>(j, "AutoLayerRuleGroup");
        x.auto_rule_def = ldtk::get_optional<ldtk::AutoLayerRuleDefinition>(j, "AutoRuleDef");
        x.custom_command = ldtk::get_optional<ldtk::LdtkCustomCommand>(j, "CustomCommand");
        x.definitions = ldtk::get_optional<ldtk::Definitions>(j, "Definitions");
        x.entity_def = ldtk::get_optional<ldtk::EntityDefinition>(j, "EntityDef");
        x.entity_instance = ldtk::get_optional<ldtk::EntityInstance>(j, "EntityInstance");
        x.entity_reference_infos = ldtk::get_optional<ldtk::FieldInstanceEntityReference>(j, "EntityReferenceInfos");
        x.enum_def = ldtk::get_optional<ldtk::EnumDefinition>(j, "EnumDef");
        x.enum_def_values = ldtk::get_optional<ldtk::EnumValueDefinition>(j, "EnumDefValues");
        x.enum_tag_value = ldtk::get_optional<ldtk::EnumTagValue>(j, "EnumTagValue");
        x.field_def = ldtk::get_optional<ldtk::FieldDefinition>(j, "FieldDef");
        x.field_instance = ldtk::get_optional<ldtk::FieldInstance>(j, "FieldInstance");
        x.grid_point = ldtk::get_optional<ldtk::FieldInstanceGridPoint>(j, "GridPoint");
        x.int_grid_value_def = ldtk::get_optional<ldtk::IntGridValueDefinition>(j, "IntGridValueDef");
        x.int_grid_value_instance = ldtk::get_optional<ldtk::IntGridValueInstance>(j, "IntGridValueInstance");
        x.layer_def = ldtk::get_optional<ldtk::LayerDefinition>(j, "LayerDef");
        x.layer_instance = ldtk::get_optional<ldtk::LayerInstance>(j, "LayerInstance");
        x.level = ldtk::get_optional<ldtk::Level>(j, "Level");
        x.level_bg_pos_infos = ldtk::get_optional<ldtk::LevelBackgroundPosition>(j, "LevelBgPosInfos");
        x.neighbour_level = ldtk::get_optional<ldtk::NeighbourLevel>(j, "NeighbourLevel");
        x.tile = ldtk::get_optional<ldtk::TileInstance>(j, "Tile");
        x.tile_custom_metadata = ldtk::get_optional<ldtk::TileCustomMetadata>(j, "TileCustomMetadata");
        x.tileset_def = ldtk::get_optional<ldtk::TilesetDefinition>(j, "TilesetDef");
        x.tileset_rect = ldtk::get_optional<ldtk::TilesetRectangle>(j, "TilesetRect");
        x.world = ldtk::get_optional<ldtk::World>(j, "World");
    }

    inline void to_json(json & j, const ldtk::ForcedRefs & x) {
        j = json::object();
        j["AutoLayerRuleGroup"] = x.auto_layer_rule_group;
        j["AutoRuleDef"] = x.auto_rule_def;
        j["CustomCommand"] = x.custom_command;
        j["Definitions"] = x.definitions;
        j["EntityDef"] = x.entity_def;
        j["EntityInstance"] = x.entity_instance;
        j["EntityReferenceInfos"] = x.entity_reference_infos;
        j["EnumDef"] = x.enum_def;
        j["EnumDefValues"] = x.enum_def_values;
        j["EnumTagValue"] = x.enum_tag_value;
        j["FieldDef"] = x.field_def;
        j["FieldInstance"] = x.field_instance;
        j["GridPoint"] = x.grid_point;
        j["IntGridValueDef"] = x.int_grid_value_def;
        j["IntGridValueInstance"] = x.int_grid_value_instance;
        j["LayerDef"] = x.layer_def;
        j["LayerInstance"] = x.layer_instance;
        j["Level"] = x.level;
        j["LevelBgPosInfos"] = x.level_bg_pos_infos;
        j["NeighbourLevel"] = x.neighbour_level;
        j["Tile"] = x.tile;
        j["TileCustomMetadata"] = x.tile_custom_metadata;
        j["TilesetDef"] = x.tileset_def;
        j["TilesetRect"] = x.tileset_rect;
        j["World"] = x.world;
    }

    inline void from_json(const json & j, ldtk::Project& x) {
        x.forced_refs = ldtk::get_optional<ldtk::ForcedRefs>(j, "__FORCED_REFS");
        x.bg_color = j.at("bgColor").get<std::string>();
        x.custom_commands = j.at("customCommands").get<std::vector<ldtk::LdtkCustomCommand>>();
        x.defs = j.at("defs").get<ldtk::Definitions>();
        x.external_levels = j.at("externalLevels").get<bool>();
        x.iid = j.at("iid").get<std::string>();
        x.json_version = j.at("jsonVersion").get<std::string>();
        x.levels = j.at("levels").get<std::vector<ldtk::Level>>();
        x.world_grid_height = ldtk::get_optional<int64_t>(j, "worldGridHeight");
        x.world_grid_width = ldtk::get_optional<int64_t>(j, "worldGridWidth");
        x.world_layout = ldtk::get_optional<ldtk::WorldLayout>(j, "worldLayout");
        x.worlds = j.at("worlds").get<std::vector<ldtk::World>>();
    }

    inline void to_json(json & j, const ldtk::Project & x) {
        j = json::object();
        j["__FORCED_REFS"] = x.forced_refs;
        j["bgColor"] = x.bg_color;
        j["customCommands"] = x.custom_commands;
        j["defs"] = x.defs;
        j["externalLevels"] = x.external_levels;
        j["iid"] = x.iid;
        j["jsonVersion"] = x.json_version;
        j["levels"] = x.levels;
        j["worldGridHeight"] = x.world_grid_height;
        j["worldGridWidth"] = x.world_grid_width;
        j["worldLayout"] = x.world_layout;
        j["worlds"] = x.worlds;
    }

    inline void from_json(const json & j, ldtk::When & x) {
        if (j == "AfterLoad") x = ldtk::When::AFTER_LOAD;
        else if (j == "AfterSave") x = ldtk::When::AFTER_SAVE;
        else if (j == "BeforeSave") x = ldtk::When::BEFORE_SAVE;
        else if (j == "Manual") x = ldtk::When::MANUAL;
        else throw "Input JSON does not conform to schema";
    }

    inline void to_json(json & j, const ldtk::When & x) {
        switch (x) {
            case ldtk::When::AFTER_LOAD: j = "AfterLoad"; break;
            case ldtk::When::AFTER_SAVE: j = "AfterSave"; break;
            case ldtk::When::BEFORE_SAVE: j = "BeforeSave"; break;
            case ldtk::When::MANUAL: j = "Manual"; break;
            default: throw "This should not happen";
        }
    }

    inline void from_json(const json & j, ldtk::TileRenderMode & x) {
        if (j == "Cover") x = ldtk::TileRenderMode::COVER;
        else if (j == "FitInside") x = ldtk::TileRenderMode::FIT_INSIDE;
        else if (j == "FullSizeCropped") x = ldtk::TileRenderMode::FULL_SIZE_CROPPED;
        else if (j == "FullSizeUncropped") x = ldtk::TileRenderMode::FULL_SIZE_UNCROPPED;
        else if (j == "NineSlice") x = ldtk::TileRenderMode::NINE_SLICE;
        else if (j == "Repeat") x = ldtk::TileRenderMode::REPEAT;
        else if (j == "Stretch") x = ldtk::TileRenderMode::STRETCH;
        else throw "Input JSON does not conform to schema";
    }

    inline void to_json(json & j, const ldtk::TileRenderMode & x) {
        switch (x) {
            case ldtk::TileRenderMode::COVER: j = "Cover"; break;
            case ldtk::TileRenderMode::FIT_INSIDE: j = "FitInside"; break;
            case ldtk::TileRenderMode::FULL_SIZE_CROPPED: j = "FullSizeCropped"; break;
            case ldtk::TileRenderMode::FULL_SIZE_UNCROPPED: j = "FullSizeUncropped"; break;
            case ldtk::TileRenderMode::NINE_SLICE: j = "NineSlice"; break;
            case ldtk::TileRenderMode::REPEAT: j = "Repeat"; break;
            case ldtk::TileRenderMode::STRETCH: j = "Stretch"; break;
            default: throw "This should not happen";
        }
    }

    inline void from_json(const json & j, ldtk::EmbedAtlas & x) {
        if (j == "LdtkIcons") x = ldtk::EmbedAtlas::LDTK_ICONS;
        else throw "Input JSON does not conform to schema";
    }

    inline void to_json(json & j, const ldtk::EmbedAtlas & x) {
        switch (x) {
            case ldtk::EmbedAtlas::LDTK_ICONS: j = "LdtkIcons"; break;
            default: throw "This should not happen";
        }
    }

    inline void from_json(const json & j, ldtk::WorldLayout & x) {
        if (j == "Free") x = ldtk::WorldLayout::FREE;
        else if (j == "GridVania") x = ldtk::WorldLayout::GRID_VANIA;
        else if (j == "LinearHorizontal") x = ldtk::WorldLayout::LINEAR_HORIZONTAL;
        else if (j == "LinearVertical") x = ldtk::WorldLayout::LINEAR_VERTICAL;
        else throw "Input JSON does not conform to schema";
    }

    inline void to_json(json & j, const ldtk::WorldLayout & x) {
        switch (x) {
            case ldtk::WorldLayout::FREE: j = "Free"; break;
            case ldtk::WorldLayout::GRID_VANIA: j = "GridVania"; break;
            case ldtk::WorldLayout::LINEAR_HORIZONTAL: j = "LinearHorizontal"; break;
            case ldtk::WorldLayout::LINEAR_VERTICAL: j = "LinearVertical"; break;
            default: throw "This should not happen";
        }
    }
}
