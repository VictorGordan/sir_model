// Victor Gordan
#include <iostream>
#include <string>
#include "imgui.h"
#include "imgui-SFML.h"
#include "SFML/Graphics.hpp"



// Window properties
const unsigned WIDTH_WINDOW = 1600;
const unsigned HEIGHT_WINDOW = 900;
const sf::String NAME_WINDOW = "SIR Model";

// Simulation properties
int speed = 60;
const unsigned NUM_CELLS = 500;
float initialInfectious = 0.00001f;
float infectionChance = 1.0f;
// One time step is equal to one day
int infectiousTime = 14;
int resistantTime = 240;

// Image to be used as topography
const char* TOPOGRAPHY_FILE = "maps/england.png";

// Colors for cells
sf::Color COLOR_SUSCEPTIBLE = sf::Color(86, 178, 114, 255);
sf::Color COLOR_INFECTIOUS = sf::Color(216, 36, 58, 255);
sf::Color COLOR_RESISTANT = sf::Color(43, 118, 112, 255);
sf::Color COLOR_WATER = sf::Color(30, 42, 66, 255);



enum States
{
	Susceptible,
	Infectious,
	Resistant
};

struct Cell
{
	unsigned char state;
	float popDensity;
	unsigned int time;
	Cell* NW;
	Cell* N;
	Cell* NE;				//   Neighbours
	Cell* W;				//   NW  N   NE
	Cell* E;				//   W  cell E 
	Cell* SW;				//   SW  S   SE
	Cell* S;
	Cell* SE;
};

// The field that was displayed on the last frame
Cell prevField[NUM_CELLS][NUM_CELLS];
// The field that will be displayed on this frame
Cell crntField[NUM_CELLS][NUM_CELLS];

// Create font object
sf::Font font;
// Holds the step you are on
unsigned int counter = 0;
sf::Text counterText;


// Generates a random float number between 0 and 1
float randf()
{
	return (float)(rand() / (float)RAND_MAX);
}
// Initializes the field of cells
void initializeFields(sf::Texture topography)
{
	// Cell that will be the border to the field of cells
	Cell resistantForPointer = { Resistant, 0.0f, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	Cell* RESISTANT = &resistantForPointer;

	// Make Image of topography to read easily
	sf::Image topographyCopy = topography.copyToImage();
	// Initialize states, population density, and times
	for (unsigned int i = 0; i < NUM_CELLS; i++)
	{
		for (unsigned int j = 0; j < NUM_CELLS; j++)
		{
			if (randf() < initialInfectious && (float)topographyCopy.getPixel(i, j).r / 255.0f != 0.0f)
			{
				prevField[i][j] = Cell{ Infectious, (float)topographyCopy.getPixel(i, j).r / 255.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
				crntField[i][j] = Cell{ Infectious, (float)topographyCopy.getPixel(i, j).r / 255.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
			}
			else if ((float)topographyCopy.getPixel(i, j).r / 255.0f == 0.0f)
			{
				prevField[i][j] = Cell{ Resistant, 0.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
				crntField[i][j] = Cell{ Resistant, 0.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
			}
			else
			{
				prevField[i][j] = Cell{ Susceptible, (float)topographyCopy.getPixel(i, j).r / 255.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
				crntField[i][j] = Cell{ Susceptible, (float)topographyCopy.getPixel(i, j).r / 255.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
			}
		}
	}
	// Initialize the neighbours
	for (unsigned int i = 0; i < NUM_CELLS; i++)
	{
		for (unsigned int j = 0; j < NUM_CELLS; j++)
		{
			if (i >= 1 && j >= 1)
			{
				prevField[i][j].NW = &prevField[i - 1][j - 1];
				crntField[i][j].NW = &crntField[i - 1][j - 1];
			}
			if (j >= 1)
			{
				prevField[i][j].N = &prevField[i][j - 1];
				crntField[i][j].N = &crntField[i][j - 1];
			}
			if (i + 1 < NUM_CELLS && j >= 1)
			{
				prevField[i][j].NE = &prevField[i + 1][j - 1];
				crntField[i][j].NE = &crntField[i + 1][j - 1];
			}
			if (i >= 1)
			{
				prevField[i][j].W = &prevField[i - 1][j];
				crntField[i][j].W = &crntField[i - 1][j];
			}
			if (i + 1 < NUM_CELLS)
			{
				prevField[i][j].E = &prevField[i + 1][j];
				crntField[i][j].E = &crntField[i + 1][j];
			}
			if (i >= 1 && j + 1 < NUM_CELLS)
			{
				prevField[i][j].SW = &prevField[i - 1][j + 1];
				crntField[i][j].SW = &crntField[i - 1][j + 1];
			}
			if (j + 1 < NUM_CELLS)
			{
				prevField[i][j].S = &prevField[i][j + 1];
				crntField[i][j].S = &crntField[i][j + 1];
			}
			if (i + 1 < NUM_CELLS && j + 1 < NUM_CELLS)
			{
				prevField[i][j].SE = &prevField[i + 1][j + 1];
				crntField[i][j].SE = &crntField[i + 1][j + 1];
			}
		}
	}
}
// Set the colors of an image to correspond with the properties of a field of cells
void imprintField(sf::Image* image)
{
	for (unsigned int i = 0; i < NUM_CELLS; i++)
	{
		for (unsigned int j = 0; j < NUM_CELLS; j++)
		{
			if (crntField[i][j].popDensity == 0.0f)
			{
				image->setPixel(i, j, COLOR_WATER);
			}
			else if (crntField[i][j].state == Susceptible)
			{
				image->setPixel(i, j, COLOR_SUSCEPTIBLE);
			}
			else if (crntField[i][j].state == Infectious)
			{
				image->setPixel(i, j, COLOR_INFECTIOUS);
			}
			else
			{
				image->setPixel(i, j, COLOR_RESISTANT);
			}
		}
	}
}
// Check if there are any infectious cells as neighbours
bool checkInfectious(unsigned int i, unsigned int j)
{
	if (prevField[i][j].NW->state == Infectious)
		return true;
	else if (prevField[i][j].N->state == Infectious)
		return true;
	else if (prevField[i][j].NE->state == Infectious)
		return true;
	else if (prevField[i][j].W->state == Infectious)
		return true;
	else if (prevField[i][j].E->state == Infectious)
		return true;
	else if (prevField[i][j].SW->state == Infectious)
		return true;
	else if (prevField[i][j].S->state == Infectious)
		return true;
	else if (prevField[i][j].SE->state == Infectious)
		return true;
	else
		return false;
}
// Handle updating of cells
void updateField()
{
	for (unsigned int i = 0; i < NUM_CELLS; i++)
	{
		for (unsigned int j = 0; j < NUM_CELLS; j++)
		{
			if (prevField[i][j].state == Susceptible && checkInfectious(i, j) && randf() < infectionChance * prevField[i][j].popDensity)
			{
				crntField[i][j].state = Infectious;
			}
			else if (prevField[i][j].state == Infectious)
			{
				if (prevField[i][j].time >= infectiousTime && randf() > prevField[i][j].popDensity) // Longer infection in bigger cities
				{
					crntField[i][j].state = Resistant;
					crntField[i][j].time = 0;
				}
				else
				{
					crntField[i][j].time++;
				}
			}
			else if (prevField[i][j].state == Resistant && prevField[i][j].popDensity != 0.0f)
			{
				if (prevField[i][j].time >= resistantTime && randf() < prevField[i][j].popDensity) // Shorter resistance in bigger cities
				{
					crntField[i][j].state = Susceptible;
					crntField[i][j].time = 0;
				}
				else
				{
					crntField[i][j].time++;
				}
			}
		}
	}
}
// Make prevField equal crntField
void swapFields()
{
	for (unsigned int i = 0; i < NUM_CELLS; i++)
	{
		for (unsigned int j = 0; j < NUM_CELLS; j++)
		{
			prevField[i][j].state = crntField[i][j].state;
			prevField[i][j].popDensity = crntField[i][j].popDensity;
			prevField[i][j].time = crntField[i][j].time;
		}
	}
}

// Create Texture object
sf::Texture topography;
// Create Sprite object to draw the texture
sf::Sprite topographySprite;
// Create Image object
sf::Image fieldImage;
// Create Texture object
sf::Texture fieldTex;
// Create Sprite object to draw the texture
sf::Sprite fieldSprite;
// Initializes all textures by assigning them the corresponding images
void initializeTextures()
{
	// Load Texture
	if (!topography.loadFromFile(TOPOGRAPHY_FILE))
	{
		std::cout << "Failed to load the topography texture" << std::endl;
	}
	// Disable smoothing
	topography.setSmooth(false);


	// Assign the texture to the sprite object
	topographySprite.setTexture(topography);
	// Center sprite
	topographySprite.setPosition(sf::Vector2f(WIDTH_WINDOW * 4.5 / 16, HEIGHT_WINDOW * 1 / 9));
	// Resize sprite
	float topographyScaleResize = ((float)WIDTH_WINDOW * 7.0f) / (16.0f * (float)topography.getSize().x);
	topographySprite.setScale(topographyScaleResize, topographyScaleResize);



	// Initialize the fields
	initializeFields(topography);

	// Make the field white
	fieldImage.create(NUM_CELLS, NUM_CELLS, sf::Color(0, 0, 0));
	// Imprint the crntField on the fieldImage
	imprintField(&fieldImage);

	// Make the Texture blank
	if (!fieldTex.create(NUM_CELLS, NUM_CELLS))
	{
		std::cout << "Failed to create the texture of the field" << std::endl;
	}
	// Disable smoothing
	fieldTex.setSmooth(false);
	// Impose the field onto the texture
	fieldTex.update(fieldImage);


	// Assign the texture to the sprite object
	fieldSprite.setTexture(fieldTex);
	// Center sprite
	fieldSprite.setPosition(sf::Vector2f(WIDTH_WINDOW * 4.5 / 16, HEIGHT_WINDOW * 1 / 9));
	// Resize sprite
	float fieldScaleResize = ((float)WIDTH_WINDOW * 7.0f) / (16.0f * (float)fieldTex.getSize().x);
	fieldSprite.setScale(fieldScaleResize, fieldScaleResize);
}

int main()
{
	// Create Window object
	sf::RenderWindow window;
	// Assign window object properties
	window.create(sf::VideoMode(WIDTH_WINDOW, HEIGHT_WINDOW), NAME_WINDOW);
	// Set Framerate Speed
	window.setFramerateLimit(speed);
	// Initialize ImGUI
	ImGui::SFML::Init(window);
	// Initialize all textures
	initializeTextures();

	// Loads font
	if (!font.loadFromFile("fonts/Roboto-Bold.ttf"))
	{
		std::cout << "Loading of font failed!" << std::endl;
	}
	// Configures settings for the counter
	counterText.setFont(font);
	counterText.setCharacterSize(24);
	counterText.setFillColor(sf::Color::White);



	// Controls the pausing and unpausing of the simulation
	bool paused = false;
	// Controls the restartation of the simulation
	bool restartSimulation = false;

	// Clock needed for ImGUI
	sf::Clock deltaClock;
	// Times used to control speed of simulation
	sf::Clock cycleClock;
	sf::Time lastTime = deltaClock.getElapsedTime();

	// Main loop
	while (window.isOpen())
	{
		// Event handling
		sf::Event event;
		while (window.pollEvent(event))
		{
			// Handle ImGUI events
			ImGui::SFML::ProcessEvent(event);

			// Handle closing of window
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::P)
				{
					paused = !paused;
				}
				if (event.key.code == sf::Keyboard::R)
				{
					// Initialize all textures
					initializeTextures();
					// Reset counter
					counter = 0;
				}
			}
		}

		// Update ImGUI UI
		ImGui::SFML::Update(window, deltaClock.restart());



		// ImGUI UI
		ImGui::Begin("Settings");
		// Handles restarting
		if (ImGui::Button("Restart"))
		{
			// Initialize all textures
			initializeTextures();
			// Reset counter
			counter = 0;
		}
		// Handles pausing
		ImGui::Checkbox("Pause", &paused);
		// Controls Variables
		ImGui::SliderInt("Speed", &speed, 1, 120);
		ImGui::SliderFloat("Initial Infectious", &initialInfectious, 0.000001f, 1.0f, "%.6f");
		ImGui::SliderFloat("Infection Chance", &infectionChance, 0.01f, 1.0f, "%.2f");
		ImGui::SliderInt("Infectious Time", &infectiousTime, 0, 365);
		ImGui::SliderInt("Resistant Time", &resistantTime, 0, 365);
		ImGui::End();



		if (!paused && cycleClock.getElapsedTime().asSeconds() - lastTime.asSeconds() >= 1.0f / speed)
		{
			// Update times
			lastTime = cycleClock.getElapsedTime();
			// Update all cells
			updateField();
			// Imprint the crntField on the fieldImage
			imprintField(&fieldImage);
			// Impose the field onto the texture
			fieldTex.update(fieldImage);

			// Clear window
			window.clear(sf::Color(25, 25, 30));
			// Displays step on which you are on
			std::string counterString = "Step: " + std::to_string(counter++);
			counterText.setString(counterString);
			window.draw(counterText);
			// Draw topography
			window.draw(topographySprite);
			// Draw field
			window.draw(fieldSprite);
			// Render ImGUI UI
			ImGui::SFML::Render(window);
			// Swap buffers
			window.display();
			// Swap fields
			swapFields();
		}
		else
		{
			// Imprint the crntField on the fieldImage
			imprintField(&fieldImage);
			// Impose the field onto the texture
			fieldTex.update(fieldImage);
			// Clear window
			window.clear(sf::Color(25, 25, 30));
			// Displays step on which you are on
			window.draw(counterText);
			// Draw topography
			window.draw(topographySprite);
			// Draw field
			window.draw(fieldSprite);
			// Render ImGUI UI
			ImGui::SFML::Render(window);
			// Swap buffers
			window.display();
			// Swap fields
			swapFields();
		}
	}

	// End ImGUI
	ImGui::SFML::Shutdown();

	return 0;
}