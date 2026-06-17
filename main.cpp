#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#define _CRT_SECURE_NO_WARNINGS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>
#include <optional>

#define TARGET_FPS 20

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define GRID_UPPER_MARGIN 50
#define GRID_LEFT_MARGIN 100

#define EDITOR_IMAGES 4

FILE* file;

enum collision_type {
	
	NONE = 0,
	INSIDE = 1,
	OUTSIDE = 2
	
};

struct ui_button {

	SDL_FRect rect;

};

struct node {

    float x, y;

};

struct collision_cluster {

    std::vector<struct node> node_array;
    enum collision_type collision;    

};

struct trigger_cluster {

    std::vector<struct node> node_array;
    std::string destination_map_name;

};

struct map_data {
	
	int number_of_clusters;
	std::vector<struct collision_cluster> clusters;
	
	int number_of_triggers;
	std::vector<struct trigger_cluster> triggers;
	
	int player_starting_x;
	int player_starting_y;
	int player_starting_angle;
	
	std::string music_file;
	
};

int write_cfg(std::string map_name, 
    std::vector<struct collision_cluster> cluster_array, 
    std::vector<struct trigger_cluster> trigger_array,
    std::string music_file,
    int playerx, int playery, int player_angle) {
	
	//fopen_s(&file, ("maps/" + map_name + "/" + map_name + ".cfg").c_str(), "w");
	file = fopen(("maps/" + map_name + "/" + map_name + ".cfg").c_str(), "w");
	if (file == NULL)
		return -1;

    fprintf(file, "CLUSTERS %d - NUMBER OF CLUSTERS. BELOW IS EACH CLUSTER CSV AND ITS COLLISION TYPE (0 = NO COLLISION, 1 AND 2 FACE OPPOSITE WAYS)\n", (int)cluster_array.size());
	for (int i = 0; i < cluster_array.size(); i++) {
		fprintf(file, "clusters/cluster_%d.csv %d\n", i + 1, cluster_array[i].collision);
	}

    fprintf(file, "\nTRIGGERS %d - NUMBER OF TRIGGERS. BELOW IS EACH TRIGGER CSV AND ITS DESTINATION\n", (int)trigger_array.size());
	for (int i = 0; i < trigger_array.size(); i++) {
		if (!trigger_array[i].destination_map_name.empty())
			fprintf(file, "triggers/trigger_%d.csv %s\n", i + 1, trigger_array[i].destination_map_name.c_str());
		else
			fprintf(file, "triggers/trigger_%d.csv 0\n", i + 1);
	}

    if (!music_file.empty()) {
		fprintf(file, "\nMUSIC 1 - PLACED INSIDE THE MUSIC FOLDER\n");
		fprintf(file, "music/%s.mp3\n", music_file.c_str());
	} else
		fprintf(file, "\nMUSIC 0 - PLACED INSIDE THE MUSIC FOLDER\n");

    fprintf(file, "\nPLAYER SPAWN - X Y ANGLE\n");
	fprintf(file, "%d %d %d", playerx, playery, -1 * player_angle);
	
	fclose(file);

    return 0;

}

int write_cluster_file(std::string filename, struct collision_cluster cluster) {

    //fopen_s(&file, filename.c_str(), "w");
	file = fopen(filename.c_str(), "w");

    if (file == NULL)
        return -1;

	for (int i = 0; i < cluster.node_array.size(); i++) {
		fprintf(file, "%d, %d\n", (int) cluster.node_array[i].x, (int) cluster.node_array[i].y);
	}

	fclose(file);

    SDL_Log(ANSI_COLOR_GREEN "Successfully wrote to file %s wahoo" ANSI_COLOR_RESET, filename.c_str());

	return 0;

}

struct collision_cluster read_cluster_file(std::string filename, enum collision_type collision) {
	
	SDL_Log("Getting file %s...", filename.c_str());
	file = fopen(filename.c_str(), "r");
	if (file == NULL) {
		SDL_Log(ANSI_COLOR_RED "Couldn't find file: %s" ANSI_COLOR_RESET, SDL_GetError());
	}
		
	struct collision_cluster new_cluster;
	new_cluster.collision = collision;
		
	char line[50];
	float num1;
	float num2;
	int current_node = 0;
	while (fgets(line, sizeof(line), file)) {
	
		sscanf(line, "%f, %f", &num1, &num2);
		new_cluster.node_array.push_back({num1, num2});
		
	}
	
	fclose(file);
	
	return new_cluster;
	
}

std::vector<struct collision_cluster> read_all_clusters(std::string map_name, int collision[], int num_of_clusters) {
	
	struct collision_cluster temp_cluster;
	std::string cluster_filename;
	
	std::vector<struct collision_cluster> cluster_array;
	
	SDL_Log("Reading the cluster files...");
	for (int i = 1; i < num_of_clusters + 1; i++) {
		
		cluster_filename = "maps/";
		cluster_filename = cluster_filename + map_name + "/clusters/" + map_name + "_cluster_" + std::to_string(i) + ".csv";
		
		if (collision[i - 1] == 0) {
			temp_cluster = read_cluster_file(cluster_filename, NONE);
			SDL_Log("Cluster %d created with collision type NONE", i - 1);
		}
		else if (collision[i - 1] == 1) {
			temp_cluster = read_cluster_file(cluster_filename, INSIDE);
			SDL_Log("Cluster %d created with collision type INSIDE", i - 1);
		}
		else if (collision[i - 1] == 2) {
			temp_cluster = read_cluster_file(cluster_filename, OUTSIDE);
			SDL_Log("Cluster %d created with collision type OUTSIDE", i - 1);
		}
		
		cluster_array.push_back(temp_cluster);
		
	}
	
	return cluster_array;
	
}

struct trigger_cluster read_trigger_file(std::string filename, std::string destination) {
	
	SDL_Log("Getting file %s...", filename.c_str());
	file = fopen(filename.c_str(), "r");
	if (file == NULL) {
		SDL_Log(ANSI_COLOR_RED "Couldn't find file: %s" ANSI_COLOR_RESET, SDL_GetError());
	}
		
	struct trigger_cluster new_trigger;
	new_trigger.destination_map_name = destination;
		
	char line[50];
	float num1;
	float num2;
	int current_node = 0;
	while (fgets(line, sizeof(line), file)) {
	
		sscanf(line, "%f, %f", &num1, &num2);
		new_trigger.node_array.push_back({num1, num2});
		
	}
	
	fclose(file);
	
	return new_trigger;
	
}

std::vector<struct trigger_cluster> read_all_triggers(std::string map_name, std::vector<std::string> destination, int num_of_clusters) {
	
	struct trigger_cluster temp_cluster;
	std::string cluster_filename;
	
	std::vector<struct trigger_cluster> trigger_array;
	
	SDL_Log("Reading the trigger files of the %d triggers...", num_of_clusters);
	for (int i = 1; i < num_of_clusters + 1; i++) {
		
		cluster_filename = "maps/";
		cluster_filename = cluster_filename + map_name + "/triggers/" + map_name + "_trigger_" + std::to_string(i) + ".csv";

		temp_cluster = read_trigger_file(cluster_filename, destination[i - 1]);

		trigger_array.push_back(temp_cluster);
		
	}
	
	return trigger_array;
	
}

int write_all_clusters(std::string map_name, std::vector<struct collision_cluster> cluster_array) {

	std::string clusters_dir = "maps/" + map_name + "/clusters";

	for (int i = 0; i < cluster_array.size(); i++) {

		std::string cluster_file_name = clusters_dir + "/" + map_name + "_cluster_" + std::to_string(i + 1) + ".csv";

		if (write_cluster_file(cluster_file_name, cluster_array[i]) != 0) {
			SDL_Log(ANSI_COLOR_RED "Error saving collision cluster %d sumn happend idk" ANSI_COLOR_RESET, i + 1);
			return -1;
		}

	}

	return 0;

}

int write_trigger_file(std::string filename, struct trigger_cluster cluster) {

    //fopen_s(&file, filename.c_str(), "w");
	file = fopen(filename.c_str(), "w");

    if (file == NULL)
        return -1;

    for (int i = 0; i < cluster.node_array.size(); i++) {
        fprintf(file, "%d, %d\n", (int)cluster.node_array[i].x, (int)cluster.node_array[i].y);
    }

    fclose(file);

    SDL_Log(ANSI_COLOR_GREEN "Successfully wrote to file %s wahoo" ANSI_COLOR_RESET, filename.c_str());

    return 0;

}

int write_all_triggers(std::string map_name, std::vector<struct trigger_cluster> trigger_array) {

    std::string clusters_dir = "maps/" + map_name + "/triggers";

    for (int i = 0; i < trigger_array.size(); i++) {

        std::string cluster_file_name = clusters_dir + "/" + map_name + "_trigger_" + std::to_string(i + 1) + ".csv";

        if (write_trigger_file(cluster_file_name, trigger_array[i]) != 0) {
            SDL_Log(ANSI_COLOR_RED "Error saving trigger cluster %d sumn happend idk" ANSI_COLOR_RESET, i + 1);
            return -1;
        }

    }

    return 0;

}

int frameStart = 0;

std::optional <std::string> text_query(SDL_Renderer* renderer, std::string prompt) {

    bool finished = false;
    std::string input = "";
    
    TTF_Font* font = TTF_OpenFont("ui/octin_spraypaint_a.ttf", 50);
    if (!font) {
		SDL_Log(ANSI_COLOR_RED "Couldn't load font file: %s" ANSI_COLOR_RESET, SDL_GetError());
		return std::nullopt;
	} else 
		SDL_Log(ANSI_COLOR_GREEN "Successfully loaded font file" ANSI_COLOR_RESET);
		
    SDL_Color text_color = { 255, 255, 255, 255 };
    SDL_Surface* temp_surface = TTF_RenderText_Solid(font, prompt.c_str(), prompt.length(), text_color);
    SDL_Texture* prompt_texture = SDL_CreateTextureFromSurface(renderer, temp_surface);
    SDL_DestroySurface(temp_surface);
    
    float prompt_size_x;
    float prompt_size_y;
    SDL_GetTextureSize(prompt_texture, &prompt_size_x, &prompt_size_y);
    SDL_FRect prompt_rect = {WINDOW_WIDTH / 2 - prompt_size_x / 2, WINDOW_HEIGHT * 2 / 5, prompt_size_x, prompt_size_y};
    
    float input_size_x;
    float input_size_y;
    SDL_FRect input_rect;
    SDL_Texture* input_texture = nullptr;


	while (!finished) {

        frameStart = SDL_GetTicks();

        SDL_SetRenderDrawColorFloat(renderer, 0.4, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
        SDL_RenderClear(renderer);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
            switch (event.type) {
				
			case SDL_EVENT_QUIT:
				
				TTF_CloseFont(font);
				
				SDL_Quit();
				exit(0);
				break;

			case SDL_EVENT_KEY_UP:

				if (event.key.scancode >= 4 && event.key.scancode < 30) { // Letters
                    input = input + (char)(event.key.key - 32);
                    SDL_Log("Current string: %s", input.c_str());
                } else if (event.key.scancode >= 30 && event.key.scancode < 40) { // Numbers
                    input = input + (char)(event.key.key);
                    SDL_Log("Current string: %s", input.c_str());
                } else if (event.key.scancode == SDL_SCANCODE_SPACE) { // Space
					input = input + (char)95;
                    SDL_Log("Current string: %s", input.c_str());
                } else if (event.key.scancode == SDL_SCANCODE_RETURN) {
					TTF_CloseFont(font);
					
					if (input.empty())
						return std::nullopt;
					
					return input;
				} else if (event.key.scancode == SDL_SCANCODE_BACKSPACE) {
					if (!input.empty())
						input.pop_back();
				}
			    
				temp_surface = TTF_RenderText_Solid(font, input.c_str(), input.length(), text_color);
				SDL_DestroyTexture(input_texture);
				input_texture = SDL_CreateTextureFromSurface(renderer, temp_surface);
				SDL_DestroySurface(temp_surface);
				SDL_GetTextureSize(input_texture, &input_size_x, &input_size_y);

                // Stupid formatting because fucking windows is having a cow. 
                input_rect.x = WINDOW_WIDTH / 2 - input_size_x / 2;
                input_rect.y = WINDOW_HEIGHT / 2;
                input_rect.w = input_size_x;
                input_rect.h = input_size_y;
                input_rect = SDL_FRect{ WINDOW_WIDTH / 2 - input_size_x / 2, WINDOW_HEIGHT / 2, input_size_x, input_size_y };

                break;

            }
            
		}
		
		SDL_RenderTexture(renderer, prompt_texture, NULL, &prompt_rect);
		SDL_RenderTexture(renderer, input_texture, NULL, &input_rect);
		
        SDL_RenderPresent(renderer);

        if (SDL_GetTicks() - frameStart < (1000 / TARGET_FPS)) {
            SDL_Delay((1000 / TARGET_FPS) - (SDL_GetTicks() - frameStart)); // cap frames
        }

	}

	SDL_RenderPresent(renderer);
	
	TTF_CloseFont(font);

    return input;

}

int make_map_dir(std::string map_name) {

    std::string full_path = "maps/" + map_name;
    SDL_CreateDirectory(full_path.c_str());

	full_path += "/clusters";
	SDL_CreateDirectory(full_path.c_str());

	full_path = "maps/" + map_name + "/triggers";
	SDL_CreateDirectory(full_path.c_str());

	full_path = "maps/" + map_name + "/music";
	SDL_CreateDirectory(full_path.c_str());

    return 0;

}

struct map_data get_map_data(std::string map_name) {

	std::string cfg_name = "maps/";
	cfg_name = cfg_name + map_name + "/" + map_name + ".cfg";
	
	SDL_Log("Loading map %s...", map_name.c_str());
	SDL_Log("Finding cfg file %s...", cfg_name.c_str());
	
	file = fopen(cfg_name.c_str(), "r");
	if (!file) {
		SDL_Log(ANSI_COLOR_RED "Couldn't load font file: %s" ANSI_COLOR_RESET, SDL_GetError());
	}
	
	char line[256];
	int num_clusters;
	char cluster_files[1024][256]; // all the cluster csv files
	int collision[1024]; // all the collision types for the clusters
	
	int num_triggers;
	char trigger_files[1024][256]; // all the cluster csv files
	char temp_destination[256];
	std::vector <std::string> trigger_destinations; // all the destinations for the triggers
	
	int music_questionmark;
	char music_file[256] = ""; // all the music files
	
	int player_x;
	int player_y;
	int player_angle;
	
	SDL_Log("Reading lines...");
	
	fgets(line, sizeof(line), file);
	SDL_Log(line);
	sscanf(line, "CLUSTERS %d", &num_clusters);
	SDL_Log("Number of clusters: %d", num_clusters);
	
	// Getting cluster file names and collision types
	for (int i = 0; i < num_clusters; i++) {
		
		fgets(line, sizeof(line), file);
		SDL_Log(line);
		sscanf(line, "%255s %d", cluster_files[i], &collision[i]);
		SDL_Log("Cluster %d: %s has collision type %d", i, cluster_files[i], collision[i]);
		
	}
	
	fgets(line, sizeof(line), file);
	
	fgets(line, sizeof(line), file);
	SDL_Log(line);
	sscanf(line, "TRIGGERS %d", &num_triggers);
	SDL_Log("Number of triggers: %d", num_triggers);
	
	// Getting cluster file names and collision types
	for (int i = 0; i < num_triggers; i++) {
		
		fgets(line, sizeof(line), file);
		SDL_Log(line);
		sscanf(line, "%255s %s", trigger_files[i], temp_destination);
		trigger_destinations.push_back(std::string(temp_destination));
		SDL_Log("Trigger %d: %s has destination %s", i, trigger_files[i], trigger_destinations[i].c_str());
		
	}
	
	fgets(line, sizeof(line), file);

	fgets(line, sizeof(line), file);
	SDL_Log(line);
	sscanf(line, "MUSIC %d", &music_questionmark);
	if (music_questionmark == 0)
		SDL_Log("There will be no music. :(");
	else {
		sscanf(line, "%s", music_file);
		SDL_Log("There will be music: %s", music_file);
	}
	
	fgets(line, sizeof(line), file);
	fgets(line, sizeof(line), file);
	
	fgets(line, sizeof(line), file);
	SDL_Log(line);
	sscanf(line, "%d %d %d", &player_x, &player_y, &player_angle);
	SDL_Log("The player will start at %d, %d and be oriented %d degrees.", player_x, player_y, player_angle);
	
	fclose(file);
	
	SDL_Log("Making new map_data object...");
	struct map_data new_map_data;
	new_map_data.number_of_clusters = num_clusters;
	new_map_data.number_of_triggers = num_triggers;
	new_map_data.player_starting_x = player_x;
	new_map_data.player_starting_y = player_y;
	new_map_data.player_starting_angle = player_angle;
	
	SDL_Log("Grabbing the clusters...");
	new_map_data.clusters = read_all_clusters(map_name, collision, num_clusters);
	new_map_data.triggers = read_all_triggers(map_name, trigger_destinations, num_triggers);
	
	new_map_data.music_file = std::string(music_file);
	
	return new_map_data;
	
}

/* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
MIX_Mixer* mixer = NULL;

char ui_image_files[EDITOR_IMAGES][256] = {
	"ui/hemorrhage_actionbar.png",
	"ui/hemorrhage_toolbar.png",
	"ui/player_start_pos.png",
	"ui/arrow.png"
};

SDL_Texture* ui_textures[EDITOR_IMAGES] = {
    NULL,
    NULL,
    NULL,
    NULL
};

std::vector<struct ui_button> ui_buttons;

int active_tool = -1;
int active_cluster = -1;
int active_trigger_cluster = -1;

std::vector<struct collision_cluster> collision_cluster_array;
std::vector<struct trigger_cluster> trigger_cluster_array;

bool player_position_selection_state = true; 
int player_start_angle = 0;
int player_start_x = 800;
int player_start_y = 450;

std::string map_music_file = "";

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{

    SDL_SetAppMetadata("Hemorrhengine", "0.1", "com.narok.template"); // Sets app metadata

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log(ANSI_COLOR_RED "Couldn't initialize SDL: %s" ANSI_COLOR_RESET, SDL_GetError());
        return SDL_APP_FAILURE;
    }
    else {
        SDL_Log(ANSI_COLOR_GREEN "Successfully initialized SDL!" ANSI_COLOR_RESET);
    }

    // Initialize Mixer
    if (!MIX_Init()) {
        SDL_Log(ANSI_COLOR_RED "Couldn't initialize SDL_Mixer: %s" ANSI_COLOR_RESET, SDL_GetError());
        return SDL_APP_FAILURE;
    }
    else {
        SDL_Log(ANSI_COLOR_GREEN "Initialized SDL_Mixer!" ANSI_COLOR_RESET);
    }

    // Initialize ttf
    if (!TTF_Init()) {
        SDL_Log(ANSI_COLOR_RED "Couldn't initialize SDL_ttf: %s" ANSI_COLOR_RESET, SDL_GetError());
        return SDL_APP_FAILURE;
    }
    else {
        SDL_Log(ANSI_COLOR_GREEN "Initialized SDL_ttf!" ANSI_COLOR_RESET);
    }

    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (mixer == NULL) {
        SDL_Log(ANSI_COLOR_RED "Couldn't initialize the mixer: %s" ANSI_COLOR_RESET, SDL_GetError());
        return SDL_APP_FAILURE;
    }
    else {
        SDL_Log(ANSI_COLOR_GREEN "Initialized the mixer!" ANSI_COLOR_RESET);
    }

    // Create window
    if (!SDL_CreateWindowAndRenderer("Hemorrhengine", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_INPUT_FOCUS, &window, &renderer)) {
        SDL_Log(ANSI_COLOR_RED "Couldn't create window/renderer: %s" ANSI_COLOR_RESET, SDL_GetError());
        return SDL_APP_FAILURE;
    }
    else {
        SDL_Log(ANSI_COLOR_GREEN "Created a window!" ANSI_COLOR_RESET);
    }

    // Sets a logical presentation for the renderer. This is super awesome and cool.
    // It scales stuff for the DPI screens and stuff
    if (!SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        SDL_Log(ANSI_COLOR_RED "SetRenderLogicalPresentation() failed: %s" ANSI_COLOR_RESET, SDL_GetError());
        return SDL_APP_FAILURE;
    }
    else {
        SDL_Log(ANSI_COLOR_GREEN "SetRenderLogicalPresentation() succeeded!" ANSI_COLOR_RESET);
    }

    for (int i = 0; i < EDITOR_IMAGES; i++) {

		SDL_Surface* temp_surface = IMG_Load(ui_image_files[i]);
		if (temp_surface == NULL) {
			SDL_Log(ANSI_COLOR_RED "Couldn't load image %s: %s" ANSI_COLOR_RESET, ui_image_files[i], SDL_GetError());
			return SDL_APP_FAILURE;
		}
		else {
			SDL_Log(ANSI_COLOR_GREEN "Loaded image %s!" ANSI_COLOR_RESET, ui_image_files[i]);
		}

		ui_textures[i] = SDL_CreateTextureFromSurface(renderer, temp_surface);
		if (ui_textures[i] == NULL) {
			SDL_Log(ANSI_COLOR_RED "Couldn't create texture for image %s: %s" ANSI_COLOR_RESET, ui_image_files[i], SDL_GetError());
			return SDL_APP_FAILURE;
		}
		else {
			SDL_Log(ANSI_COLOR_GREEN "Created texture for image %s!" ANSI_COLOR_RESET, ui_image_files[i]);
		}

		SDL_DestroySurface(temp_surface);

    }

    ui_buttons.push_back({ { 0, 200, 100, 50 } }); // add collision cluster
    ui_buttons.push_back({ { 0, 250, 100, 50 } }); // remove cluster
    ui_buttons.push_back({ { 0, 300, 100, 50 } }); // add node
    ui_buttons.push_back({ { 0, 350, 100, 50 } }); // toggle collision mode

    ui_buttons.push_back({ { 0, 600, 100, 50 } }); // add trigger cluster
    ui_buttons.push_back({ { 0, 650, 100, 50 } }); // remove trigger cluster
    ui_buttons.push_back({ { 0, 700, 100, 50 } }); // add trigger node
    ui_buttons.push_back({ { 0, 750, 100, 50 } }); // change trigger action

    ui_buttons.push_back({ { 240, 0, 95, 50 } }); // save
    ui_buttons.push_back({ { 500, 0, 95, 50 } }); // load
    ui_buttons.push_back({ { 740, 0, 75, 50 } }); // run
    ui_buttons.push_back({ { 950, 0, 110, 50 } }); // music
	ui_buttons.push_back({ { 1196, 0, 250, 50 } });

    if (ui_buttons.empty()) {
        SDL_Log(ANSI_COLOR_RED "Failed to allocate for buttons. %s" ANSI_COLOR_RESET, SDL_GetError());
        return SDL_APP_FAILURE;
    }
    else {
        SDL_Log(ANSI_COLOR_GREEN "Created da buttons!" ANSI_COLOR_RESET);
    }

    return SDL_APP_CONTINUE;

}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{

    struct collision_cluster temp_cluster;
    struct trigger_cluster temp_trigger;

    std::string map_name;
    std::string clusters_dir;
    std::string cluster_file_name;
    
    switch (event->type) {

	case SDL_EVENT_QUIT:
		SDL_Log(ANSI_COLOR_GREEN "Quittin time" ANSI_COLOR_RESET);
        return SDL_APP_SUCCESS;
		break;
    
	case SDL_EVENT_MOUSE_BUTTON_UP:
        int mouse_x = event->button.x;
        int mouse_y = event->button.y;
		if (event->button.button == SDL_BUTTON_LEFT) {
			
			SDL_Log("Mouse: %d, %d", mouse_x, mouse_y);
			
            if (mouse_x > GRID_LEFT_MARGIN && mouse_y > GRID_UPPER_MARGIN) {

                switch (active_tool) {

                case 0: // Create new collision cluster (tool)

					temp_cluster = collision_cluster();
					temp_cluster.collision = INSIDE;
					temp_cluster.node_array.push_back({ (float)mouse_x - mouse_x % 10, (float)mouse_y - mouse_y % 10 });
                    collision_cluster_array.push_back(temp_cluster);
                    SDL_Log("Adding new node at (%d, %d)", mouse_x, mouse_y);

					active_cluster = collision_cluster_array.size() - 1;
                    active_trigger_cluster = -1;

                    break;

                case 1: // Delete selected collision cluster (tool)

					if (active_cluster != -1)
					    collision_cluster_array.erase(collision_cluster_array.begin() + active_cluster);

                    active_cluster = -1;
                    active_trigger_cluster = -1;

                    break;

                case 2: // Add new collision node to selected cluster (tool)

					if (active_cluster != -1) {

                        collision_cluster_array[active_cluster].node_array.push_back({ (float)mouse_x - mouse_x % 10, (float)mouse_y - mouse_y % 10 });
                        SDL_Log("Adding new node at (%d, %d)", mouse_x, mouse_y);
                
				    }

                    active_trigger_cluster = -1;

                    break;

                case 4: // Create new trigger cluster (tool)

                    temp_trigger = trigger_cluster();
                    temp_trigger.destination_map_name = "";
                    temp_trigger.node_array.push_back({ (float)mouse_x - mouse_x % 10, (float)mouse_y - mouse_y % 10 });
                    trigger_cluster_array.push_back(temp_trigger); // Create an initial node to start the cluster off
                    SDL_Log("Adding new trigger node at (%d, %d)", mouse_x, mouse_y);

                    active_trigger_cluster = trigger_cluster_array.size() - 1;
                    active_cluster = -1;

                    break;

                case 5: // Delete selected trigger cluster (tool)

                    if (active_trigger_cluster != -1)
                        trigger_cluster_array.erase(trigger_cluster_array.begin() + active_trigger_cluster);

                    active_cluster = -1;
                    active_trigger_cluster = -1;

                    break;

                case 6: // Add node to trigger cluster (tool)

                    if (active_trigger_cluster != -1) {

                        trigger_cluster_array[active_trigger_cluster].node_array.push_back({ (float)mouse_x - mouse_x % 10, (float)mouse_y - mouse_y % 10 });
                        SDL_Log("Adding new trigger node at (%d, %d)", mouse_x, mouse_y);

                    }

                    active_cluster = -1;

                    break;

                case 12: // Change player starting position / angle (tool)
						// Selects player position on first click, player angle on second.
                
					// 0 if the user is selecting the x/y location. Will switch to false when angle is being selected
					if (player_position_selection_state == true) {
						
						player_start_x = mouse_x;
						player_start_y = mouse_y;
						player_position_selection_state = false;
						
					} else {
						
						player_start_angle = 180 / 3.14 * SDL_atan2(mouse_y - player_start_y, mouse_x - player_start_x);
						player_position_selection_state = true;
						
					}					

                    break;

                default:
                    break;

                }

            }
            else {

				for (int i = 0; i < ui_buttons.size(); i++) {
					if (mouse_x >= ui_buttons[i].rect.x && mouse_x <= ui_buttons[i].rect.x + ui_buttons[i].rect.w &&
						mouse_y >= ui_buttons[i].rect.y && mouse_y <= ui_buttons[i].rect.y + ui_buttons[i].rect.h) {
						active_tool = i;
						SDL_Log("Clicked button %d", i);
						break;
					}
				}

                switch (active_tool) {
					
				case 3: // Switch collision type for current cluster (action)
				
					if (active_cluster != -1)
						switch (collision_cluster_array[active_cluster].collision) {
							
						case NONE:
							collision_cluster_array[active_cluster].collision = INSIDE;
							break;
						
						case INSIDE:
							collision_cluster_array[active_cluster].collision = OUTSIDE;
							break;
							
						case OUTSIDE:
							collision_cluster_array[active_cluster].collision = NONE;
							break;
							
						}
				
					break;
					
				case 7: // Select trigger destination for current trigger cluster (action)
						
				if (active_trigger_cluster != -1) {
						
					auto result = text_query(renderer, "Enter current trigger's destination's map name:");
					if (!result.has_value()) {
						SDL_Log(ANSI_COLOR_RED "Empty string returned. Trigger will lead nowhere.:-(" ANSI_COLOR_RESET);
						break;
					}

					// Set the trigger's destination to whatever the user entered
					trigger_cluster_array[active_trigger_cluster].destination_map_name = result.value();
                    
				} else { SDL_Log("No trigger actively selected."); }

                    break;

                case 8: { // Save da map, used brackets to keep auto's scope isolated here (action)

					auto result = text_query(renderer, "Enter map name to save to:");
					if (!result.has_value()) {
						SDL_Log(ANSI_COLOR_RED "Empty string returned. :-(" ANSI_COLOR_RESET);
						break;
					}

                    SDL_Log(ANSI_COLOR_GREEN "Creating map folder..." ANSI_COLOR_RESET);
                    map_name = result.value();
                    make_map_dir(map_name); // Create map dir and sub dirs for clusters, trigs and muisca
                    SDL_Log(ANSI_COLOR_GREEN "Creating map file..." ANSI_COLOR_RESET);
					write_cfg(map_name, collision_cluster_array, trigger_cluster_array, map_music_file, (int)player_start_x, (int)player_start_y, (int)player_start_angle); // Create map config file
					write_all_clusters(map_name, collision_cluster_array);
					write_all_triggers(map_name, trigger_cluster_array);
				}

                    break;
                    
                 case 9: { // Load da map (action)
                 
					auto result = text_query(renderer, "Enter map to load:");
                 
					struct map_data map_cfg = get_map_data(result.value());
					collision_cluster_array = map_cfg.clusters;
					trigger_cluster_array = map_cfg.triggers;
					player_start_x = map_cfg.player_starting_x;
					player_start_y = map_cfg.player_starting_y;
					player_start_angle = map_cfg.player_starting_angle;
					map_music_file = map_cfg.music_file;
                 
					break;
					
				}
				
				case 11: { // Pick music file (action)
					
					auto result = text_query(renderer, "Enter MP3 file to load as this map's music file.");
					if (!result.has_value()) {
						SDL_Log(ANSI_COLOR_RED "Empty string returned. :-(" ANSI_COLOR_RESET);
						break;
					}
					
					map_music_file = result.value();
					SDL_Log(ANSI_COLOR_GREEN "Changed music file to: %s" ANSI_COLOR_RESET, map_music_file);
					
					break;
					
					// Modify this map's music file ig
					
				}
				
				case 12: // Pick player starting location (action)
				
					player_position_selection_state = true;
					
					break;

                }
            }

		}
		else if (event->button.button == SDL_BUTTON_RIGHT) {
			
			bool cluster_selected = false;

			for (int i = 0; i < collision_cluster_array.size(); i++) {
				for (int j = 0; j < collision_cluster_array[i].node_array.size(); j++) {
					float node_x = collision_cluster_array[i].node_array[j].x;
					float node_y = collision_cluster_array[i].node_array[j].y;
					if (mouse_x >= node_x - 5 && mouse_x <= node_x + 5 &&
						mouse_y >= node_y - 5 && mouse_y <= node_y + 5) {

						active_cluster = i;
                        active_trigger_cluster = -1;
                        cluster_selected = true;

						SDL_Log("Selected cluster at (%f, %f)", node_x, node_y);
						break;
					}
				}
			}

            for (int i = 0; i < trigger_cluster_array.size(); i++) {
                for (int j = 0; j < trigger_cluster_array[i].node_array.size(); j++) {
                    float node_x = trigger_cluster_array[i].node_array[j].x;
                    float node_y = trigger_cluster_array[i].node_array[j].y;
                    if (mouse_x >= node_x - 5 && mouse_x <= node_x + 5 &&
                        mouse_y >= node_y - 5 && mouse_y <= node_y + 5) {

                        active_cluster = -1;
                        active_trigger_cluster = i;
						cluster_selected = true;

                        SDL_Log("Selected triggercluster at (%f, %f)", node_x, node_y);
                        break;
                    }
                }
            }

			if (!cluster_selected) {
				active_cluster = -1;
				active_trigger_cluster = -1;
			}

		}
		break;


    }

    return SDL_APP_CONTINUE; 
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{

    frameStart = SDL_GetTicks();

    SDL_SetRenderDrawColorFloat(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);

    SDL_RenderClear(renderer);

    for (int i = GRID_UPPER_MARGIN; i < 1600; i += 100) {

        SDL_SetRenderDrawColorFloat(renderer, 0.4, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
        for (int j = 0; j < 100; j += 10) {

            SDL_RenderLine(renderer, 0, i + j, WINDOW_WIDTH, i + j);
            SDL_RenderLine(renderer, i + j - 50, 0, i + j - 50, WINDOW_HEIGHT);

        }

        SDL_SetRenderDrawColorFloat(renderer, 1, 1, 1, SDL_ALPHA_OPAQUE_FLOAT);
        SDL_RenderLine(renderer, 0, i, WINDOW_WIDTH, i);
        SDL_RenderLine(renderer, i - 50, 0, i - 50, WINDOW_HEIGHT);

        SDL_SetRenderDrawColorFloat(renderer, 0.8, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
        SDL_RenderLine(renderer, 0, i + 50, WINDOW_WIDTH, i + 50);
        SDL_RenderLine(renderer, i, 0, i, WINDOW_HEIGHT);

    }

    SDL_FRect actionbar_rect = { 0, 0, WINDOW_WIDTH, 50 };
    SDL_FRect toolbar_rect = { 0, 0, 100, WINDOW_HEIGHT };
    SDL_FRect player_start_rect = { (float) player_start_x - 50, (float) player_start_y - 50, 100, 100 };
    SDL_FPoint player_start_rotation_point = { 50, 50 };
    SDL_RenderTexture(renderer, ui_textures[0], NULL, &actionbar_rect);
    SDL_RenderTexture(renderer, ui_textures[1], NULL, &toolbar_rect);
    SDL_RenderTextureRotated(renderer, ui_textures[2], NULL, &player_start_rect, player_start_angle, &player_start_rotation_point, SDL_FLIP_NONE);

    // This for loop is where the program draws the collision clusters.
    for (int i = 0; i < collision_cluster_array.size(); i++) {

        for (int j = 0; j < collision_cluster_array[i].node_array.size(); j++) {

			if (i == active_cluster) // if this is the active cluster, draw it in yellow
                SDL_SetRenderDrawColorFloat(renderer, 1, 1, 0, SDL_ALPHA_OPAQUE_FLOAT);
            else // otherwise, use red
                SDL_SetRenderDrawColorFloat(renderer, 1, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);

            SDL_FRect node_rect = { collision_cluster_array[i].node_array[j].x - 5, collision_cluster_array[i].node_array[j].y - 5, 10, 10 };
            SDL_RenderRect(renderer, &node_rect);
            if (collision_cluster_array[i].node_array.size() > 0) {

                if (j == 0) {
                    int array_size = collision_cluster_array[i].node_array.size();
                    SDL_RenderLine(renderer, collision_cluster_array[i].node_array[array_size - 1].x, collision_cluster_array[i].node_array[array_size - 1].y, collision_cluster_array[i].node_array[0].x, collision_cluster_array[i].node_array[0].y);
                }
                else
                    SDL_RenderLine(renderer, collision_cluster_array[i].node_array[j - 1].x, collision_cluster_array[i].node_array[j - 1].y, collision_cluster_array[i].node_array[j].x, collision_cluster_array[i].node_array[j].y);
			
            }
            
        }
        
        // Drawing the collision-type-indicating arrows
        if (collision_cluster_array[i].node_array.size() > 1 && collision_cluster_array[i].collision != NONE) {
			
			SDL_FRect arrow_rect;
			SDL_GetTextureSize(ui_textures[3], &arrow_rect.w, &arrow_rect.h);
			
			float distance_between_points_y;
			float distance_between_points_x;
			float angle;
			SDL_FPoint center_point = {0, 0};
			
			// Draw the arrows representing the collision type
			for (int j = 0; j < collision_cluster_array[i].node_array.size() - 1; j++) {
					
				distance_between_points_y = collision_cluster_array[i].node_array[j + 1].y - collision_cluster_array[i].node_array[j].y;
				distance_between_points_x = collision_cluster_array[i].node_array[j + 1].x - collision_cluster_array[i].node_array[j].x;
				
				arrow_rect.x = collision_cluster_array[i].node_array[j + 1].x - distance_between_points_x / 2;
				arrow_rect.y = collision_cluster_array[i].node_array[j + 1].y - distance_between_points_y / 2;
				
				if (collision_cluster_array[i].collision == INSIDE)
					angle = SDL_atan2f(distance_between_points_y, distance_between_points_x) * 180 / 3.14 - 90;
				else if (collision_cluster_array[i].collision == OUTSIDE)
					angle = SDL_atan2f(distance_between_points_y, distance_between_points_x) * 180 / 3.14 + 90;
				
				SDL_RenderTextureRotated(renderer, ui_textures[3], NULL, &arrow_rect, angle, &center_point, SDL_FLIP_NONE);

			}
			
			// Draw the last arrow on the line between the first and last node
			
			distance_between_points_y = collision_cluster_array[i].node_array[0].y - collision_cluster_array[i].node_array[collision_cluster_array[i].node_array.size() - 1].y;
			distance_between_points_x = collision_cluster_array[i].node_array[0].x - collision_cluster_array[i].node_array[collision_cluster_array[i].node_array.size() - 1].x;
			
			arrow_rect.x = collision_cluster_array[i].node_array[0].x - distance_between_points_x / 2;
			arrow_rect.y = collision_cluster_array[i].node_array[0].y - distance_between_points_y / 2;
			
			if (collision_cluster_array[i].collision == INSIDE)
				angle = SDL_atan2f(distance_between_points_y, distance_between_points_x) * 180 / 3.14 - 90;
			else if (collision_cluster_array[i].collision == OUTSIDE)
				angle = SDL_atan2f(distance_between_points_y, distance_between_points_x) * 180 / 3.14 + 90;
			
			SDL_RenderTextureRotated(renderer, ui_textures[3], NULL, &arrow_rect, angle, &center_point, SDL_FLIP_NONE);
			
		}

    }


    // Drawing the trigger clusters
    for (int i = 0; i < trigger_cluster_array.size(); i++) {

        for (int j = 0; j < trigger_cluster_array[i].node_array.size(); j++) {

            if (i == active_trigger_cluster) // if this is the active cluster, draw it in yellow
                SDL_SetRenderDrawColorFloat(renderer, 1, 1, 0, SDL_ALPHA_OPAQUE_FLOAT);
            else // otherwise, use blue
                SDL_SetRenderDrawColorFloat(renderer, 0, 0, 1, SDL_ALPHA_OPAQUE_FLOAT);

            SDL_FRect node_rect = { trigger_cluster_array[i].node_array[j].x - 5, trigger_cluster_array[i].node_array[j].y - 5, 10, 10 };
            SDL_RenderRect(renderer, &node_rect);
            if (trigger_cluster_array[i].node_array.size() > 0) {

                if (j == 0) {
                    int array_size = trigger_cluster_array[i].node_array.size();
                    SDL_RenderLine(renderer, trigger_cluster_array[i].node_array[array_size - 1].x, trigger_cluster_array[i].node_array[array_size - 1].y, trigger_cluster_array[i].node_array[0].x, trigger_cluster_array[i].node_array[0].y);
                }
                else
                    SDL_RenderLine(renderer, trigger_cluster_array[i].node_array[j - 1].x, trigger_cluster_array[i].node_array[j - 1].y, trigger_cluster_array[i].node_array[j].x, trigger_cluster_array[i].node_array[j].y);

            }
        }

    }


    if (active_tool != -1) {
		SDL_SetRenderDrawColorFloat(renderer, 1, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
		SDL_FRect active_button_rect = ui_buttons[active_tool].rect;
        SDL_RenderRect(renderer, &active_button_rect);
    }

    SDL_RenderPresent(renderer);

    if (SDL_GetTicks() - frameStart < (1000 / TARGET_FPS)) {
        SDL_Delay((1000 / TARGET_FPS) - (SDL_GetTicks() - frameStart)); // cap frames
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */

}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}
