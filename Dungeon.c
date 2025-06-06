#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ROOMS 100
#define MAX_DIRECTION 4

// Enums
typedef enum 
{ 
    EMPTY,
    MONSTER, 
    ITEM, 
    TREASURE 
} roomType;

typedef enum 
{ 
    GOBLIN, 
    TROLL 
} monsterType;
typedef enum 
{ 
    DAMAGE,
    HEALTH 
} itemType;

// Structs
typedef struct 
{
    monsterType type;
    int hp;
    int damage;
} monster_struct;

typedef struct 
{
    itemType type;
    int value;
} item_struct;

typedef struct player_struct
{
    int hp;
    int damage;
    int room_id;
} player_struct;

// Forward declaration
struct room_struct;

// Function pointer type
typedef void (*room_action)(player_struct*, struct room_struct*);

typedef struct room_struct 
{
    int id;
    roomType type;
    int visited;
    struct room_struct* directions[MAX_DIRECTION];
    monster_struct* monster;
    item_struct* item;
    room_action action; // <<< Function pointer
} room_struct;

// Globals
room_struct* rooms[MAX_ROOMS];
int room_count = 0;
int treasure_placed = 0;
int total_rooms = 0;


// Function declarations
room_struct* create_room(int id);
void connect_rooms(room_struct* a, room_struct* b);
void generate_dungeon(int num_rooms);
void free_dungeon();
void enter_room(player_struct* player, room_struct* room);
void fight(player_struct* player, monster_struct* monster);
void print_room_info(room_struct* room);
void save_game(player_struct* player, const char* filename);
int load_game(player_struct* player, const char* filename);


// Room actions
void action_empty(player_struct* player, room_struct* room) 
{
    printf("The room is empty.\n");
}

void action_monster(player_struct* player, room_struct* room) 
{
    fight(player, room->monster);
    free(room -> monster);
    room -> monster = NULL;
    room -> type = EMPTY;
    room -> action = action_empty;
}

void action_item(player_struct* player, room_struct* room) 
{
    if (room -> item -> type == HEALTH) 
    {
        printf("You found a health potion! (+%d hp)\n", room -> item -> value);
        player -> hp += room->item->value;
    } 
    else 
    {
        printf("You found a damage potion! (+%d dmg)\n", room->item->value);
        player->damage += room->item->value;
    }

    free(room->item);
    room -> item = NULL;
    room -> type = EMPTY;
    room -> action = action_empty;
}

void action_treasure(player_struct* player, room_struct* room) 
{
    printf("You found the treasure!!! Good job!\n");
}

// Kamers aanmaken
room_struct* create_room(int id) 
{
    room_struct* room = malloc(sizeof(room_struct));
    room -> id = id;
    room -> visited = 0;
    room -> monster = NULL;
    room -> item = NULL;
    for (int i = 0; i < MAX_DIRECTION; i++) room->directions[i] = NULL;

    if (!treasure_placed && id != 0 && ((rand() % 100) < 30 || id == total_rooms - 1)) 
    {
        room -> type = TREASURE;
        treasure_placed = 1;
        room -> action = action_treasure;
    } 
    else 
    {
        int chance = rand() % 100;
        if (chance < 30) 
        {
            room -> type = MONSTER;
            room -> monster = malloc(sizeof(monster_struct));
            room -> monster -> type = rand() % 2;
            room -> monster -> hp = 5 + rand() % 10;
            room -> monster -> damage = 2 + rand() % 3;
            room -> action = action_monster;
        } 
        else if (chance < 60) 
        {
            room -> type = ITEM;
            room -> item = malloc(sizeof(item_struct));
            room -> item->type = rand() % 2;
            room -> item->value = 3 + rand() % 3;
            room -> action = action_item;
        } 
        else 
        {
            room -> type = EMPTY;
            room -> action = action_empty;
        }
    }

    rooms[room_count++] = room;
    return room;
}

void connect_rooms(room_struct* a, room_struct* b) 
{
    for (int i = 0; i < MAX_DIRECTION; i++) 
    {
        if (a -> directions[i] == NULL) 
        {
            a -> directions[i] = b;
            break;
        }
    }
    for (int i = 0; i < MAX_DIRECTION; i++) 
    {
        if (b -> directions[i] == NULL) 
        {
            b -> directions[i] = a;
            break;
        }
    }
}

void generate_dungeon(int num_rooms) 
{
    for (int i = 0; i < num_rooms; i++) create_room(i);
    for (int i = 1; i < num_rooms; i++) 
    {
        int connect = rand() % i;
        connect_rooms(rooms[i], rooms[connect]);
    }
}

void enter_room(player_struct* player, room_struct* room) 
{
    printf("You have entered room %d\n", room->id);
    printf("Current hp: %d, dmg: %d\n", player->hp, player->damage);

    if (!room -> visited) 
    {
        room -> visited = 1;
        room -> action(player, room); // <<< execute function pointer
    }
}

void fight(player_struct* player, monster_struct* monster) 
{
    printf("A monster has appeared!!! (%d hp, %d dmg)\n", monster -> hp, monster -> damage);
    while (player -> hp > 0 && monster -> hp > 0) 
    {
        int bits = rand() % 32;
        printf("Attack pattern...");
        for (int i = 4; i >= 0; i--) 
        {
            int mask = 1 << i;
            if (bits & mask) 
            {
                monster -> hp -= player -> damage;
                printf("1");
            } 
            else 
            {
                player -> hp -= monster -> damage;
                printf("0");
            }
        }
        printf("\nPlayer HP: %d, Monster HP: %d\n", player -> hp, monster -> hp);
    }
}

void print_room_info(room_struct* room) 
{
    printf("The room has doors leading to: ");
    for (int i = 0; i < MAX_DIRECTION; i++) 
    {
        if (room->directions[i]) 
        {
            printf("%d ", room->directions[i]->id);
        }
    }
    printf("\n");
}

void save_game(player_struct* player, const char* filename) 
{
    FILE* f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "%d %d %d %d\n", player -> room_id, player -> hp, player -> damage, total_rooms);
    fclose(f);
}

int load_game(player_struct* player, const char* filename) 
{
    FILE* f = fopen(filename, "r");
    if (!f) return 0;
    int rooms;
    fscanf(f, "%d %d %d %d", &player -> room_id, &player -> hp, &player -> damage, &rooms);
    fclose(f);
    total_rooms = rooms;
    generate_dungeon(total_rooms);
    return 1;
}

void free_dungeon() 
{
    for (int i = 0; i < room_count; i++) 
    {
        if (rooms[i] -> monster) free(rooms[i] -> monster);
        if (rooms[i] -> item) free(rooms[i] -> item);
        free(rooms[i]);
    }
}

int main(int argc, char* argv[]) 
{
    srand(time(NULL));
    player_struct player = {100, 15, 0};

    if (argc < 2) 
    {
        printf("Use: %s <Number of rooms> or %s load <File>\n", argv[0], argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "load") == 0) 
    {
        if (!load_game(&player, argv[2])) 
        {
            printf("Loading failed.\n");
            return 1;
        }
    } 
    else 
    {
        int numb_rooms = atoi(argv[1]);
        total_rooms = numb_rooms;
        generate_dungeon(numb_rooms);
        player.room_id = 0;
    }

    while (1) 
    {
        room_struct* current = rooms[player.room_id];
        enter_room(&player, current);

        if (player.hp <= 0) 
        {
            printf("The player has died.\n");
            break;
        }
        if (current -> type == TREASURE) 
        {
            break;
        }

        print_room_info(current);
        printf("Choose room id: ");
        int choice;
        scanf("%d", &choice);

        int valid_choice = 0;
        for (int i = 0; i < MAX_DIRECTION; i++) 
        {
            if (current -> directions[i] && current -> directions[i] -> id == choice) 
            {
                valid_choice = 1;
                break;
            }
        }

        if (valid_choice) 
        {
            player.room_id = choice;
            printf("Do you want to save? (y/n): ");
            char save;
            scanf(" %c", &save);

            if (save == 'y') 
            {
                save_game(&player, "savegame.txt");
            }

        } 
        else 
        {
            printf("Invalid room. You can't go there!\n");
        }
    }

    free_dungeon();
    return 0;
}
