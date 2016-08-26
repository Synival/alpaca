/* defs.h
 * ------
 * common definitions shared between the client and server. */

#ifndef __ALPACA_C_DEFS_H
#define __ALPACA_C_DEFS_H

#include <stdio.h>

#include "llist.h"
#include "utils.h"

/* connection flags. */
#define CONNECTION_WRITING 0x01
#define CONNECTION_WROTE   0x02

/* server functions. */
#define SERVER_FUNC_JOIN      0
#define SERVER_FUNC_LEAVE     1
#define SERVER_FUNC_READ      2
#define SERVER_FUNC_PRE_WRITE 3
#define SERVER_FUNC_MAX       4

/* server flags. */
#define SERVER_OPEN     0x01
#define SERVER_QUIT     0x02
#define SERVER_RUNNING  0x04
#define SERVER_PIPE     0x08

/* stylesheet element flags. */
#define ELEMENT_HIDE          0x01
#define ELEMENT_SMALLER_SIZE  0x02

/* some direction constants. */
#define DIRECTION_NORTHWEST   0
#define DIRECTION_NORTH       1
#define DIRECTION_NORTHEAST   2
#define DIRECTION_WEST        3
#define DIRECTION_CENTER      4
#define DIRECTION_EAST        5
#define DIRECTION_SOUTHWEST   6
#define DIRECTION_SOUTH       7
#define DIRECTION_SOUTHEAST   8
#define DIRECTION_MAX         9

/* types for io_open(). */
#define IO_TYPE_FILE       0

/* flags for io_open(). */
#define IO_READ      0x01
#define IO_WRITE     0x02
#define IO_APPEND    0x04
#define IO_CREATE    0x08
#define IO_DOCUMENT  0x10
#define IO_OPEN      0x20
#define IO_EOF       0x40

/* types of variables used for sorting in trees. */
#define TREE_FLOAT   0
#define TREE_STRING  1

/* foreground colors. */
#define COLOR_BLACK     0x0000
#define COLOR_RED       0x0001
#define COLOR_GREEN     0x0002
#define COLOR_YELLOW    0x0003
#define COLOR_BLUE      0x0004
#define COLOR_MAGENTA   0x0005
#define COLOR_CYAN      0x0006
#define COLOR_WHITE     0x0007
#define COLOR_BOLD      0x0008
#define COLOR_BBLACK    (COLOR_BLACK   | COLOR_BOLD)
#define COLOR_BRED      (COLOR_RED     | COLOR_BOLD)
#define COLOR_BGREEN    (COLOR_GREEN   | COLOR_BOLD)
#define COLOR_BYELLOW   (COLOR_YELLOW  | COLOR_BOLD)
#define COLOR_BBLUE     (COLOR_BLUE    | COLOR_BOLD)
#define COLOR_BMAGENTA  (COLOR_MAGENTA | COLOR_BOLD)
#define COLOR_BCYAN     (COLOR_CYAN    | COLOR_BOLD)
#define COLOR_BWHITE    (COLOR_WHITE   | COLOR_BOLD)

/* background colors. */
#define COLOR_BACK_BLACK     0x0000
#define COLOR_BACK_RED       0x0010
#define COLOR_BACK_GREEN     0x0020
#define COLOR_BACK_YELLOW    0x0030
#define COLOR_BACK_BLUE      0x0040
#define COLOR_BACK_MAGENTA   0x0050
#define COLOR_BACK_CYAN      0x0060
#define COLOR_BACK_WHITE     0x0070
#define COLOR_BACK_BBLACK    (COLOR_BACK_BLACK   | COLOR_BACK_BOLD)
#define COLOR_BACK_BRED      (COLOR_BACK_RED     | COLOR_BACK_BOLD)
#define COLOR_BACK_BGREEN    (COLOR_BACK_GREEN   | COLOR_BACK_BOLD)
#define COLOR_BACK_BYELLOW   (COLOR_BACK_YELLOW  | COLOR_BACK_BOLD)
#define COLOR_BACK_BBLUE     (COLOR_BACK_BLUE    | COLOR_BACK_BOLD)
#define COLOR_BACK_BMAGENTA  (COLOR_BACK_MAGENTA | COLOR_BACK_BOLD)
#define COLOR_BACK_BCYAN     (COLOR_BACK_CYAN    | COLOR_BACK_BOLD)
#define COLOR_BACK_BWHITE    (COLOR_BACK_WHITE   | COLOR_BACK_BOLD)
#define COLOR_BACK_BOLD      0x0080

/* pathing info. */
#define PATH_MAX_ENTITY    512
#define PATH_MAX_PLAYER    2048
#define PATH_MAX_TARGETS   16

/* path node flags. */
#define PATH_NODE_OPEN     0x01
#define PATH_NODE_PORTAL   0x02

/* path generation flags. */
#define PATH_GEN_EXHAUSTIVE   0x01

/* maximum number of unique identifiers. */
#define ID_MAX 65536

/* XML stuff. */
#define XML_CDATA       0x01
#define XML_WRITE       0x02
#define XML_HIJACKED    0x04
#define XML_DOCUMENT    0x08
#define XML_MAX_DEPTH   64

/* type definitions. */
typedef struct _option_type option_type;
typedef struct _option_func_type option_func_type;
typedef unsigned int id_type;
typedef unsigned long int color_type;
typedef struct _tree_type tree_type;
typedef unsigned long int flags_type;
typedef struct _path_list_type path_list_type;
typedef struct _path_node_type path_node_type;
typedef struct _path_goal_type path_goal_type;
typedef struct _path_portal_type path_portal_type;
typedef struct _path_array_type path_array_type;
typedef struct _xml_type xml_type;
typedef struct _io_file_type io_file_type;
typedef struct _io_manifest_type io_manifest_type;
typedef struct _asset_cmd_type asset_cmd_type;
typedef struct _asset_script_type asset_script_type;
typedef struct _asset_execution_type asset_execution_type;
typedef struct _asset_func_type asset_func_type;
typedef struct _asset_read_dir_data asset_read_dir_data;
typedef struct _fivetile_unique_type fivetile_unique_type;
typedef struct _fivetile_def_type fivetile_def_type;
typedef struct _binary_tree_sort_type binary_tree_sort_type;
typedef struct _binary_sort_type binary_sort_type;
typedef struct _stylesheet_type stylesheet_type;
typedef struct _stylesheet_element_type stylesheet_element_type;
typedef struct _server_type server_type;
typedef struct _connection_type connection_type;

/* function definitions. */
typedef int option_func (int sub_command, char **arg, int args, void *data);
#define OPTION_FUNC(x) \
   int x (int sub_command, char **arg, int args, void *data)
typedef float  path_score_func (path_list_type *path, path_node_type *origin,
   id_type parent_id, int x, int y, void **data, size_t *data_size);
#define PATH_SCORE_FUNC(z) \
   float (z) (path_list_type *path, path_node_type *origin, \
   id_type parent_id, int x, int y, void **data, size_t *data_size)
typedef float  path_goal_func (path_goal_type *goal, id_type parent_id,
                               int x, int y);
#define PATH_GOAL_FUNC(z) \
   float (z) (path_goal_type *goal, id_type parent_id, int x, int y)
typedef int path_diag_func (path_list_type *path, path_node_type *origin,
   path_node_type *adj1, path_node_type *adj2, id_type parent_id,
   int x, int y);
#define PATH_DIAG_FUNC(x) \
   int (x) (path_list_type *path, path_node_type *origin, \
   path_node_type *adj1, path_node_type *adj2, id_type parent_id, \
   int x, int y)
typedef int path_portal_func (path_list_type *path, path_node_type *origin);
#define PATH_PORTAL_FUNC(x) \
   int (x) (path_list_type *path, path_node_type *origin)

#define WORLEY_FUNC(x) \
   float (x) (int x1, int y1, int x2, int y2, int res, float arg)
typedef float worley_func (int, int, int, int, int, float);

#define IO_MANIFEST_FUNC(x) \
   int (x) (char *filename, io_manifest_type *manifest, void *data)
typedef int io_manifest_func (char *, io_manifest_type *, void *);

#define ASSET_FUNC(x) \
   int x (asset_script_type *script, asset_cmd_type *cmd, \
          asset_execution_type *exe, char **arg, int args, int sub_command)
typedef int asset_func (asset_script_type *, asset_cmd_type *,
                        asset_execution_type *, char **, int, int);

#define ASSET_PAUSE_FUNC(x) \
   int x (asset_execution_type *exe, asset_script_type *script, float t, \
          void *data)
typedef int asset_pause_func (asset_execution_type *, asset_script_type *,
                              float, void *);

#define STYLE_FUNC(x) \
   int x (stylesheet_type *style, stylesheet_element_type *element)
typedef int stylesheet_func_type (stylesheet_type *style,
   stylesheet_element_type *element);

#define SERVER_FUNC(x) \
   int x (server_type *server, connection_type *connection, \
          void *data, size_t data_size)
typedef int server_func (server_type *, connection_type *, void *, size_t);

#endif
