/** Copyright (c) 2021
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * Authored by: Kris Henriksen <krishenriksen.work@gmail.com>
 *
 * AnberPorts
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengles.h>
#include <stdbool.h>
#include <dirent.h> 
#include <stdio.h>

// Screen dimension constants
const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 320;

// The window we'll be rendering to
SDL_Window* window = NULL;

// hw renderer
SDL_Renderer* renderer = NULL;

// Our game controller
SDL_Joystick* GameController = NULL;

// theme music
Mix_Music* music = NULL;

// re render
bool invalidated = false;

int back_key = 7;
int start_key = 310;
int a_key = 0;
int l1_key = 4;
int r1_key = 5;
int up_key = 1;
int down_key = 4;

typedef struct {
	char *text;
	char *desc;
	char *release;
	SDL_Rect pos;
} Button;

Button gameList[255];
Button multimediaList[255];
Button streamingList[255];
Button betaList[255];
Button systemList[255];

int game_count = -1;
int multimedia_count = -1;
int streaming_count = -1;
int beta_count = -1;
int system_count = -1;

int port_count = 0;
int current_port = 0;
int tab = 0;
int box_width = 52;
int box_height = 73;

int port_offset = 30;

char update[40];

char *ltrim(char *s) {
	while(isspace(*s)) s++;
	return s;
}

char *rtrim(char *s) {
	char* back = s + strlen(s);
	while(isspace(*--back));
	*(back+1) = '\0';
	return s;
}

char *trim(char *s) {
	return rtrim(ltrim(s));
}

void loadMusic(char path[]) {
	// load the MP3 file "theme.mp3" to play as music
	music = Mix_LoadMUS(path);
	if (!music) {
		printf("Failed to load music!: %s\n", Mix_GetError());
	}

	// If there is no music playing
	if (Mix_PlayingMusic() == 0) {
		Mix_PlayMusic(music, -1);
	}
}

/*
 * Load image at specified path
 */
void loadMedia(char path[]) {
    SDL_Surface* loadedSurface = IMG_Load(path);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
	SDL_FreeSurface(loadedSurface);

	// Apply the image stretched
	SDL_Rect stretchRect;
	stretchRect.x = 0;
	stretchRect.y = 0;
	stretchRect.w = SCREEN_WIDTH;
	stretchRect.h = SCREEN_HEIGHT;

	SDL_RenderCopy(renderer, texture, NULL, &stretchRect);
}

SDL_Color colorConverter(unsigned long hexValue) {
	SDL_Color rgbColor;
	rgbColor.r = ((hexValue >> 16) & 0xFF);	// Extract the RR byte
	rgbColor.g = ((hexValue >> 8) & 0xFF);	// Extract the GG byte
	rgbColor.b = ((hexValue) & 0xFF);		// Extract the BB byte
	rgbColor.a = 255;						// Alpha
	return rgbColor;
}

/*
 * Write out text
 */
void initTextRect(int x, int y, int w, int h, char text[], int fontSize, int hexColorFont) {
	// font
	TTF_Font* font = TTF_OpenFont("./assets/segoeui.ttf", fontSize);
	SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, colorConverter(hexColorFont));
	TTF_CloseFont(font);
	SDL_Texture* blendedTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_FreeSurface(textSurface);

	SDL_Rect pos;
	pos.x = x;
	pos.y = y;
	pos.w = w;
	pos.h = h;

	SDL_QueryTexture(blendedTexture, NULL, NULL, &pos.w, &pos.h); 
	SDL_RenderCopy(renderer, blendedTexture, NULL, &pos);
}

/*
 * Load image at specified path
 */
void initButtonImg(int x, int y, char path[]) {
	char image[100];

	strcpy(image, "./assets/boxart/");
	strcat(image, path);
	strcat(image, ".png");

    // Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(image);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);

	// Apply the image stretched
	SDL_Rect imageRect;
	imageRect.x = x;
	imageRect.y = y;
	imageRect.w = box_width;
	imageRect.h = box_height;

	//SDL_QueryTexture(texture, NULL, NULL, &imageRect.w, &imageRect.h); 
    SDL_RenderCopy(renderer, texture, NULL, &imageRect);
}

void initButtonRect(int x, int y, int w, int h, int hexColorRect, char text[], int fontSize, int hexColorFont) {
	Button button;
	button.pos.x = x;
	button.pos.y = y;
	button.pos.w = w;
	button.pos.h = h;
	button.text = text;

	SDL_Surface* backgroundSurface = SDL_CreateRGBSurface(0, w, h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

	SDL_Color rgb = colorConverter(hexColorRect);
	SDL_FillRect(backgroundSurface, NULL, SDL_MapRGBA(backgroundSurface->format, rgb.r, rgb.g, rgb.b, 200));
	SDL_Texture* backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);

	SDL_RenderCopy(renderer, backgroundTexture, NULL, &button.pos);

	// font
	TTF_Font* font = TTF_OpenFont("./assets/segoeui.ttf", fontSize);
	SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, colorConverter(hexColorFont));
	TTF_CloseFont(font);
	SDL_Texture* blendedTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_FreeSurface(textSurface);

	SDL_Rect pos;
	pos.x = x + 10;
	pos.y = y;
	pos.w = w;
	pos.h = h;

	SDL_QueryTexture(blendedTexture, NULL, NULL, &pos.w, &pos.h); 
	SDL_RenderCopy(renderer, blendedTexture, NULL, &pos);
}

void menu(int offset) {
    // Load sponsor image
	loadMedia("./assets/sponsor.png");

	initButtonRect(0, 0, 70, 30, 0x629ae7, "Games", 18, (tab == 1 ? 0xFFFFFF : 0x000000));
	initButtonRect(70, 0, 110, 30, 0x629ae7, "Multimedia", 18, (tab == 2 ? 0xFFFFFF : 0x000000));
	initButtonRect(180, 0, 140, 30, 0x629ae7, "Streaming", 18, (tab == 3 ? 0xFFFFFF : 0x000000));
	initButtonRect(320, 0, 80, 30, 0x629ae7, "Beta", 18, (tab == 4 ? 0xFFFFFF : 0x000000));
	initButtonRect(400, 0, 80, 30, 0x629ae7, "System", 18, (tab == 5 ? 0xFFFFFF : 0x000000));

	int offset_end = (offset + 4);

	int w = (SCREEN_WIDTH - box_width);
	int h = (box_height / 3);

	if (tab == 1) {
		port_count = game_count;
	}
	else if (tab == 2) {
		port_count = multimedia_count;
	}
	else if (tab == 3) {
		port_count = streaming_count;	
	}
	else if (tab == 4) {
		port_count = beta_count;	
	}
	else if (tab == 5) {
		port_count = system_count;	
	}

	if (offset_end > port_count) {
		offset_end = port_count;
	}

	for (int i = offset; i <= offset_end; i++) {
		int y = (box_height * i - (offset * box_height)) + port_offset;
		int y1 = (box_height * i - (offset * box_height)) + (box_height / 3) + port_offset;
		int y2 = (box_height * i - (offset * box_height)) + (box_height / 3) + port_offset + 24;

		if (tab == 1) {
			// image
			initButtonImg(0, y, gameList[i].text);
			// headline
			initButtonRect(gameList[i].pos.x, y, w, h, 0x858585, gameList[i].text, 18, 0x000000);
			// description
			initButtonRect(gameList[i].pos.x, y1, w, h, 0x858585, gameList[i].desc, 14, 0x000000);
			// release
			initButtonRect(gameList[i].pos.x, y2, w, h+1, 0x858585, gameList[i].release, 12, 0x000000);
			// selected
			initButtonImg(0, port_offset, "install");
		}
		else if (tab == 2) {
			// image
			initButtonImg(0, y, multimediaList[i].text);
			// headline
			initButtonRect(multimediaList[i].pos.x, y, w, h, 0x858585, multimediaList[i].text, 18, 0x000000);
			// description
			initButtonRect(multimediaList[i].pos.x, y1, w, h, 0x858585, multimediaList[i].desc, 14, 0x000000);
			// release
			initButtonRect(multimediaList[i].pos.x, y2, w, h+1, 0x858585, multimediaList[i].release, 12, 0x000000);
			// selected
			initButtonImg(0, port_offset, "install");
		}
		else if (tab == 3) {
			// image
			initButtonImg(0, y, streamingList[i].text);
			// headline
			initButtonRect(streamingList[i].pos.x, y, w, h, 0x858585, streamingList[i].text, 18, 0x000000);
			// description
			initButtonRect(streamingList[i].pos.x, y1, w, h, 0x858585, streamingList[i].desc, 14, 0x000000);
			// release
			initButtonRect(streamingList[i].pos.x, y2, w, h+1, 0x858585, streamingList[i].release, 12, 0x000000);
			// selected
			initButtonImg(0, port_offset, "install");
		}
		else if (tab == 4) {
			// image
			initButtonImg(0, y, betaList[i].text);
			// headline
			initButtonRect(betaList[i].pos.x, y, w, h, 0x858585, betaList[i].text, 18, 0x000000);
			// description
			initButtonRect(betaList[i].pos.x, y1, w, h, 0x858585, betaList[i].desc, 14, 0x000000);
			// release
			initButtonRect(betaList[i].pos.x, y2, w, h+1, 0x858585, betaList[i].release, 12, 0x000000);
			// selected
			initButtonImg(0, port_offset, "install");
		}
		else if (tab == 5) {
			// image
			initButtonImg(0, y, systemList[i].text);
			// headline
			initButtonRect(systemList[i].pos.x, y, w, h, 0x858585, systemList[i].text, 18, 0x000000);
			// description
			initButtonRect(systemList[i].pos.x, y1, w, h, 0x858585, systemList[i].desc, 14, 0x000000);
			// release
			initButtonRect(systemList[i].pos.x, y2, w, h+1, 0x858585, systemList[i].release, 12, 0x000000);
			// selected
			initButtonImg(0, port_offset, "install");
		}
	}

	invalidated = true;
}

void install() {
	if (tab > 0 && port_count >= 0) {
		char command[100];

		switch (tab) {
			case 1: {
				strcpy(command, "bash ./scripts/games/");
				strcat(command, gameList[current_port].text);
				break;
			}

			case 2: {
				strcpy(command, "bash ./scripts/multimedia/");
				strcat(command, multimediaList[current_port].text);
				break;
			}

			case 3: {
				strcpy(command, "bash ./scripts/streaming/");
				strcat(command, streamingList[current_port].text);
				break;
			}

			case 4: {
				strcpy(command, "bash ./scripts/beta/");
				strcat(command, betaList[current_port].text);
				break;
			}

			case 5: {
				strcpy(command, "bash ./scripts/system/");
				strcat(command, systemList[current_port].text);
				break;
			}
		}

		loadMedia("./assets/pleasewait.png");

		// Render the changes above
		SDL_RenderPresent(renderer);

		FILE *fp;
		char path[1035];

		/* Open the command for reading. */
		fp = popen(command, "r");

		/* Read the output a line at a time - output it. */
		while (fgets(path, sizeof(path), fp) != NULL) {
			printf("%s\n", trim(path));
		}

		/* close */
		pclose(fp);

		loadMedia("./assets/pleasewait.png");

    	initTextRect(130, 20, 320, 30, "Please see AnberPorts Wiki", 18, 0xFFFFFF);
		initTextRect(110, 50, 370, 30, "for information about Game Files", 18, 0xFFFFFF);
		initTextRect(80, 260, 370, 30, "Press Select to quit or Start to continue", 18, 0xFFFFFF);

		port_count = 0;
		current_port = 0;
		tab = 0;

		invalidated = true;
	}
}

void readDir(char dirPath[], int section) {
	int count = -1;

	DIR *d;
	struct dirent *dir;;
	d = opendir(dirPath);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (dir->d_type == DT_REG) {
				count++;

				Button button;

				button.text = malloc(strlen(dir->d_name) + 1);
				strcpy(button.text, dir->d_name);

				button.pos.x = box_width;
				button.pos.y = (count * box_height) + 30;

				// description
				char path[100];

				strcpy(path, dirPath);
				strcat(path, button.text);

				FILE *file = fopen(path, "r");
				char line[100];

				int i = 0; 
				while (fgets(line, sizeof(line), file)) {
					if (i == 1) {
						char *a = line + 2;
						button.desc = malloc(strlen(a) + 1);
						strcpy(button.desc, trim(a));
					}
					else if (i == 2) {
						char *b = line + 2;
						button.release = malloc(strlen(b) + 1);
						strcpy(button.release, trim(b));
						break;
					}

					i++;
				}

				if (section == 0) {
					gameList[count] = button;
					game_count = count;
				}
				else if (section == 1) {
					multimediaList[count] = button;
					multimedia_count = count;
				}
				else if (section == 2) {
					streamingList[count] = button;
					streaming_count = count;
				}
				else if (section == 3) {
					betaList[count] = button;
					beta_count = count;
				}
				else if (section == 4) {
					if (strcmp(button.text, "Update") == 0) {
						button.release = update;
					}

					systemList[count] = button;
					system_count = count;
				}
			}
		}
		
		closedir(d);
	}
}

static void MainExit(void) {
    // music
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	Mix_CloseAudio();
	Mix_Quit();

	// Close controller
	SDL_JoystickClose(GameController);
	GameController = NULL;

	// Destroy renderer
	SDL_DestroyRenderer(renderer);
	renderer = NULL;

    // Destroy window
    SDL_DestroyWindow(window);
    window = NULL;

    // Quit SDL subsystems
    SDL_Quit();
}

bool init() {
	// initialize SDL and Controller
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	// window
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);	
	window = SDL_CreateWindow("AnberPorts", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_RENDERER_ACCELERATED | SDL_WINDOW_FULLSCREEN);

    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	// vsync
	SDL_GL_SetSwapInterval(1);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

    // controller
	if (SDL_NumJoysticks() < 1) {
		printf("Warning: No joysticks connected!\n" );
		return false;
	}

	GameController = SDL_JoystickOpen(0);

	if (GameController == NULL) {
		printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	// font
	TTF_Init();

	int flags = MIX_INIT_OGG | MIX_INIT_MOD;
	int initted = Mix_Init(flags);
	if ((initted & flags) != flags) {
		printf("Mix_Init: Failed to init required ogg and mod support!\n");
		printf("Mix_Init: %s\n", Mix_GetError());
	}

	// Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	}

	// Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
	}

	return true;
}

int main(int argc, char* argv[]) {
	// command line arguments
	if (argc >= 2) {
		strcpy(update, argv[1]);
	}

	atexit(MainExit);

	unsigned int Frame_Starting_Time = 0, Elapsed_Time;
	SDL_Event event;

    // Start up SDL and create window
    if (!init()) {
        printf("Failed to initialize!\n");
        return EXIT_FAILURE;
    }

	// Load sponsor SDL_image
	loadMedia("./assets/sponsor.png");

	invalidated = true;

    // Load music
	loadMusic("./assets/theme.ogg");

	// populate ports
	readDir("./scripts/games/", 0);
	readDir("./scripts/multimedia/", 1);
	readDir("./scripts/streaming/", 2);
	readDir("./scripts/beta/", 3);
	readDir("./scripts/system/", 4);

	bool quit = false;

	// Process incoming SDL events
	while (!quit) {
		// Keep the time corresponding to the frame rendering beginning
		Frame_Starting_Time = SDL_GetTicks();

		while (SDL_PollEvent(&event)) {
			// buttons
			if (event.type == SDL_JOYBUTTONDOWN) {
				switch (event.jbutton.button) {
					// select
					case 7: {
						quit = true;
						system("bash ./scripts/system/Exit");
						break;
					}

					// start
					case 6: {
						port_count = 0;
						current_port = 0;
						tab = 1;
						menu(current_port);
						break;
					}

					// r1
					case 5: {
						port_count = 0;
						current_port = 0;

						if (tab < 5) {
							tab += 1;
							menu(current_port);
						}
						break;
					}

					// l1
					case 4: {
						port_count = 0;
						current_port = 0;

						if (tab > 1) {
							tab -= 1;
							menu(current_port);
						}
						break;
					}

					// a
					case 0: {
						if (tab > 0) {
							install();
						}
						else {
							port_count = 0;
							current_port = 0;
							tab = 1;
							menu(current_port);
						}
						break;
					}
				}
			}
			else if (event.type == SDL_JOYHATMOTION) {
				// d-pad
				switch (event.jhat.value) {
					case SDL_HAT_UP: {
						if (current_port > 0 && port_count > 0) {
							current_port -= 1;
							menu(current_port);
						}

						break;
					}

					case SDL_HAT_DOWN: {
						if (current_port < port_count && port_count > 0) {
							current_port += 1;
							menu(current_port);
						}

						break;
					}

					case SDL_HAT_LEFT: {
						port_count = 0;
						current_port = 0;

						if (tab > 1) {
							tab -= 1;
							menu(current_port);
							
							invalidated = true;
						}
						break;
					}

					case SDL_HAT_RIGHT: {
						port_count = 0;
						current_port = 0;

						if (tab < 5) {
							tab += 1;
							menu(current_port);

						}
						break;
					}
				}
			}
		}

		if (invalidated) {
			// Render the changes above
			SDL_RenderPresent(renderer);
			invalidated = false;
		}

		// Wait enough time to get a 60Hz refresh rate
		Elapsed_Time = SDL_GetTicks() - Frame_Starting_Time;
		if (Elapsed_Time < 16) {
			SDL_Delay(16 - Elapsed_Time);
		}
	}

	return 0;
}