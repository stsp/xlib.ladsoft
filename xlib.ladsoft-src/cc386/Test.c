#include <string.h>
#include <stdio.h>
#include <time.h>

//Function Prototypes
void SetupRooms(void);
/*
void DescribeRoom(unsigned int);
void DescribeExits(unsigned int);
unsigned int GetDirection(unsigned int);
void explanation(void);
*/

//Global #defines and enums

//enum {NORTH,EAST,SOUTH,WEST}; //Assigned with 0,1,2,3 respectively.

#define NORTH 	0
#define EAST	1
#define SOUTH	2
#define WEST	3
#define ROOM_COUNT 8 //Is the number of rooms.
#define TRUE 1
#define FALSE 0

enum {EXIT_HOUSE = 100, EXIT_PROGRAM};

// Global Structure Type Definitions -------------------------------------------
typedef struct 
{
	unsigned char Description[200]; // If 100 or 400 chars reserved instead, no problems. Weird.
	unsigned char Exit_Description[200];
	unsigned int Exit[4];
	
} LOCATION;

#define SZ_LOCATION sizeof (LOCATION) //Might need this later.

//------------------------------------------------------------------------------

//Global Location Type Variables From Struct LOCATION. 
LOCATION Room[ROOM_COUNT];

//C Functions from here...

//------------------------------------------------------------------------------
// Main Function

void main(void)
{
	unsigned int RoomNumber = ROOM_COUNT; // The starting room.
	unsigned int Direction;
	
	
	//unsigned int i;
	//unsigned char *pointer;
	
	SetupRooms();

	system("cls"); // Clear Screen
	printf("\rOK. Back here in main()");
		
/*
just some debug stuff, commented out for now...
		
	for (i = 0; i < ROOM_COUNT; i++)
	{
	   printf("size of Room[%d] is %d\n\r", i+1, sizeof(Room[i+1]));
	}
		
	printf("size of unsigned char is %d\n\r", sizeof(unsigned char));
	
	for (i = 0, pointer=Room[8].Description; i < 400; i++)
	{
	   if (*pointer == '\0') break;
			   
	   printf("%c", *pointer);
	   pointer++;
	}
*/				
   exit(0);
}

//------------------------------------------------------------------------------

void SetupRooms(void)
{
	printf("You shouldn't be seeing this message.\r");
	
	//Room 1 Def
	strcpy(Room[1].Description, "Room1: You are in the Cinema Room. The room is dark and there is a big screen on one of the walls."); // 99 chars with the slashnull terminator.
	strcpy(Room[1].Exit_Description, "Your exits are North and West.");
	Room[1].Exit[NORTH] = 5;
	Room[1].Exit[EAST] = 0;
	Room[1].Exit[SOUTH] = 0;
	Room[1].Exit[WEST] = 2;
	
	//Room 2 Def
	strcpy(Room[2].Description, "Room2: You are in the Utility Room. There is a washing machine in the corner.");
	strcpy(Room[2].Exit_Description, "Your exits are North and East.");
	Room[2].Exit[NORTH] = 4;
	Room[2].Exit[EAST] = 1;
	Room[2].Exit[SOUTH] = 0;
	Room[2].Exit[WEST] = 0;
		
	//Room 3 Def
	strcpy(Room[3].Description, "Room3: You are in the Study. There are two computers and a big bookshelf in this room.");
	strcpy(Room[3].Exit_Description, "Your only exit is East.");
	Room[3].Exit[NORTH] = 0;
	Room[3].Exit[EAST] = 4;
	Room[3].Exit[SOUTH] = 0;
	Room[3].Exit[WEST] = 0;
	
	//Room 4 Def
	strcpy(Room[4].Description, "Room4: You are in the Kitchen. Here there is an oven, hob, fridge and sink with counters.");
	strcpy(Room[4].Exit_Description, "Your exits are North, South, East and West.");
	Room[4].Exit[NORTH] = 8;
	Room[4].Exit[EAST] = 5;
	Room[4].Exit[SOUTH] = 2;
	Room[4].Exit[WEST] = 3;
	
	//Room 5 Def
	strcpy(Room[5].Description, "Room5: You are in the Conservatory. This is a very warm, sunny room with a white sofa.");
	strcpy(Room[5].Exit_Description, "Your exits are North, South and West.");
	Room[5].Exit[NORTH] = 6;
	Room[5].Exit[EAST] = 0;
	Room[5].Exit[SOUTH] = 1;
	Room[5].Exit[WEST] = 4;
	
	//Room 6 Def
	strcpy(Room[6].Description, "Room6: You are in the Dining Room. This is where the family eat their meals.");
	strcpy(Room[6].Exit_Description, "Your exits are South and West.");
	Room[6].Exit[NORTH] = 0;
	Room[6].Exit[EAST] = 0;
	Room[6].Exit[SOUTH] = 5;
	Room[6].Exit[WEST] = 8;
	
	//Room 7 Def
	strcpy(Room[7].Description, "Room7: You are in the Lounge. There are two large sofas and a big TV in the corner.");
	strcpy(Room[7].Exit_Description, "Your only exit is South.");
	Room[7].Exit[NORTH] = 0;
	Room[7].Exit[EAST] = 0;
	Room[7].Exit[SOUTH] = 8;
	Room[7].Exit[WEST] = 0;
		
	//Room 8 Def
	strcpy(Room[8].Description, "Room8: You are in the Entrance Hall. It is long and there is a shoe rack against a wall.");
	strcpy(Room[8].Exit_Description, "Your exits are North, South, East and West."); //Make this text about 10 chars shorter & there's no problem.
	Room[8].Exit[NORTH] = 7;
	Room[8].Exit[EAST] = 6; //Exit and Entrance
	Room[8].Exit[SOUTH] = 4;
	Room[8].Exit[WEST] = EXIT_HOUSE;
	
	/*
	strcpy(Room[8].Description, "Room8: You are in the Entrance Hall. It is long and there is a shoe rack against a wall.");
	strcpy(Room[8].Exit_Description, "Your exits are North, South, East and West.");
	Room[8].Exit[NORTH] = 7;
	Room[8].Exit[EAST] = 6; //Exit and Entrance
	Room[8].Exit[SOUTH] = 4;
	Room[8].Exit[WEST] = EXIT_HOUSE;
	
	*/
}

//------------------------------------------------------------------------------
