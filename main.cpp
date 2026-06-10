#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#define _CRT_SECURE_NO_WARNINGS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>

#define TARGET_FPS 20

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define GRID_UPPER_MARGIN 50
#define GRID_LEFT_MARGIN 100

#define EDITOR_IMAGES 2

FILE* file;

struct ui_button {

	SDL_FRect rect;

};

struct node {

    float x, y;

};

struct collision_cluster {

    std::vector<struct node> node_array;

};

struct trigger_cluster {

    std::vector<struct node> node_array;

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
		fprintf(file, "clusters/cluster_%d.csv 0\n", i + 1);
	}

    fprintf(file, "\nTRIGGERS %d - NUMBER OF TRIGGERS. BELOW IS EACH TRIGGER CSV AND ITS DESTINATION\n", (int)trigger_array.size());
	for (int i = 0; i < trigger_array.size(); i++) {
		fprintf(file, "triggers/trigger_%d.csv 0\n", i + 1);
	}

    fprintf(file, "\nMUSIC - PLACED INSIDE THE MUSIC FOLDER\n");
	fprintf(file, "music/%s\n", music_file.c_str());

    fprintf(file, "\nPLAYER SPAWN - X Y ANGLE\n");
	fprintf(file, "%d %d %d", playerx, playery, player_angle);
	
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

int write_all_clusters(std::string map_name, std::vector<struct collision_cluster> cluster_array) {

	std::string clusters_dir = "maps/" + map_name + "/clusters";

	for (int i = 0; i < cluster_array.size(); i++) {

		std::string cluster_file_name = clusters_dir + "/cluster_" + std::to_string(i + 1) + ".csv";

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

        std::string cluster_file_name = clusters_dir + "/trigger_" + std::to_string(i + 1) + ".csv";

        if (write_trigger_file(cluster_file_name, trigger_array[i]) != 0) {
            SDL_Log(ANSI_COLOR_RED "Error saving trigger cluster %d sumn happend idk" ANSI_COLOR_RESET, i + 1);
            return -1;
        }

    }

    return 0;

}

std::string text_query(SDL_Renderer* renderer, std::string prompt) {

    bool finished = false;

    std::string input = "";

	while (!finished) {

        SDL_SetRenderDrawColorFloat(renderer, 0.4, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
        SDL_RenderClear(renderer);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
            switch (event.type) {

			case SDL_EVENT_KEY_UP:

				printf("Key up: %c\n", event.key.key);
                if (event.key.scancode >= 4 && event.key.scancode < 40) {
                    input = input + (char)event.key.key;
                    printf("%s", input.c_str());
                }

                break;

            }
		}

		//SDL_Log(input.c_str());
        SDL_RenderPresent(renderer);

	}

	SDL_RenderPresent(renderer);

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

/* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
MIX_Mixer* mixer = NULL;

char ui_image_files[EDITOR_IMAGES][256] = {
	"ui/hemorrhage_actionbar.png",
	"ui/hemorrhage_toolbar.png"
};

SDL_Texture* ui_textures[EDITOR_IMAGES] = {
    NULL,
    NULL
};

std::vector<struct ui_button> ui_buttons;

int active_tool = -1;
int active_cluster = -1;
int active_trigger_cluster = -1;

std::vector<struct collision_cluster> collision_cluster_array;
std::vector<struct trigger_cluster> trigger_cluster_array;

int frameStart = 0;

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
			
            if (mouse_x > GRID_LEFT_MARGIN && mouse_y > GRID_UPPER_MARGIN) {

                switch (active_tool) {

                case 0: // Create new collision cluster

					temp_cluster = collision_cluster();
					temp_cluster.node_array.push_back({ (float)mouse_x - mouse_x % 10, (float)mouse_y - mouse_y % 10 });
                    collision_cluster_array.push_back(temp_cluster);
                    SDL_Log("Adding new node at (%d, %d)", mouse_x, mouse_y);

					active_cluster = collision_cluster_array.size() - 1;
                    active_trigger_cluster = -1;

                    break;

                case 1: // Delete selected collision cluster

					if (active_cluster != -1)
					    collision_cluster_array.erase(collision_cluster_array.begin() + active_cluster);

                    active_cluster = -1;
                    active_trigger_cluster = -1;

                    break;

                case 2: // Add new collision node to selected cluster

					if (active_cluster != -1) {

                        collision_cluster_array[active_cluster].node_array.push_back({ (float)mouse_x - mouse_x % 10, (float)mouse_y - mouse_y % 10 });
                        SDL_Log("Adding new node at (%d, %d)", mouse_x, mouse_y);
                
				    }

                    active_trigger_cluster = -1;

                    break;

                case 4: // Create new trigger cluster

                    temp_trigger = trigger_cluster();
                    temp_trigger.node_array.push_back({ (float)mouse_x - mouse_x % 10, (float)mouse_y - mouse_y % 10 });
                    trigger_cluster_array.push_back(temp_trigger);
                    SDL_Log("Adding new trigger node at (%d, %d)", mouse_x, mouse_y);

                    active_trigger_cluster = trigger_cluster_array.size() - 1;
                    active_cluster = -1;

                    break;

                case 5: // Delete selected trigger cluster

                    if (active_trigger_cluster != -1)
                        trigger_cluster_array.erase(trigger_cluster_array.begin() + active_trigger_cluster);

                    active_cluster = -1;
                    active_trigger_cluster = -1;

                    break;

                case 6: // Add node to trigger cluster

                    if (active_trigger_cluster != -1) {

                        trigger_cluster_array[active_trigger_cluster].node_array.push_back({ (float)mouse_x - mouse_x % 10, (float)mouse_y - mouse_y % 10 });
                        SDL_Log("Adding new trigger node at (%d, %d)", mouse_x, mouse_y);

                    }

                    active_cluster = -1;

                    break;

                case 8: // Save

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

                case 8:

					text_query(renderer, "Enter map name: ");

                    SDL_Log(ANSI_COLOR_GREEN "Creating map folder..." ANSI_COLOR_RESET);
                    map_name = "map1";
                    make_map_dir(map_name); // Create map dir and sub dirs for clusters, trigs and muisca

                    SDL_Log(ANSI_COLOR_GREEN "Creating map file..." ANSI_COLOR_RESET);
					write_cfg(map_name, collision_cluster_array, trigger_cluster_array, "doom3.mp3", 100, 100, 0); // Create map config file
					write_all_clusters(map_name, collision_cluster_array);
					write_all_triggers(map_name, trigger_cluster_array);

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
    SDL_RenderTexture(renderer, ui_textures[0], NULL, &actionbar_rect);
    SDL_RenderTexture(renderer, ui_textures[1], NULL, &toolbar_rect);

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
