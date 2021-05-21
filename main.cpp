// Victor Gordan
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "imgui.h"
#include "implot.h"
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
int seed = 1;
// One time step is equal to one day
int infectiousTime = 14;
int resistantTime = 240;

// Images to be used as the topography map
std::vector<std::string> maps;
std::string topographyFile;

// Proportion Graph variables
int totalCells = 0;
int totalSusceptible = 0;
int totalInfectious = 0;
int totalResistant = 0;
std::vector<float> graphSusceptible;
std::vector<float> graphInfectious;
std::vector<float> graphResistant;
std::vector<float> graphCounter;
bool drawSusceptible = true;
bool drawInfectious = true;
bool drawResistant = true;
bool legend = false;

// Growth Graph variables
int growthSusceptible = 0;
int growthInfectious = 0;
int growthResistant = 0;
std::vector<float> graphGrowthSusceptible;
std::vector<float> graphGrowthInfectious;
std::vector<float> graphGrowthResistant;
bool drawGrowthSusceptible = true;
bool drawGrowthInfectious = true;
bool drawGrowthResistant = true;
bool lockY = false;
bool legendGrowth = false;

// Colors for cells
float colorSusceptible[3] = { 86.0f / 255.0f, 178.0f / 255.0f, 114.0f / 255.0f};
float colorInfectious[3] = { 216.0f / 255.0f, 36.0f / 255.0f, 58.0f / 255.0f};
float colorResistant[3] = { 43.0f / 255.0f, 118.0f / 255.0f, 112.0f / 255.0f};
float colorWater[3] = { 30.0f / 255.0f, 42.0f / 255.0f, 66.0f / 255.0f};



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


// Reads all the maps
void readMaps()
{
	maps.clear();
	maps.reserve(100);
	std::string path = "maps";
	for (const auto& entry : std::filesystem::directory_iterator(path))
		maps.push_back(entry.path().string());
}

// Returns the name of a map by its path
std::string getName(std::string path)
{
	return path.substr(path.find("\\") + 1, path.find(".png") - 5);
}


// Generates a random float number between 0 and 1
float randf()
{
	return (float)(rand() / (float)RAND_MAX);
}
// Initializes the field of cells
void initializeFields(sf::Texture topography)
{
	// Set seed
	srand(seed);

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
			if (randf() < initialInfectious * ((0.8f / (1.0f + 1800.0f * std::exp(-15.0f * (float)topographyCopy.getPixel(i, j).r / 255.0f))) + 0.1f) && (float)topographyCopy.getPixel(i, j).r / 255.0f != 0.0f)
			{
				prevField[i][j] = Cell{ Infectious, (float)topographyCopy.getPixel(i, j).r / 255.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
				crntField[i][j] = Cell{ Infectious, (float)topographyCopy.getPixel(i, j).r / 255.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
				totalInfectious++;
				totalCells++;
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
				totalSusceptible++;
				totalCells++;
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
	// Initialize graph
	graphSusceptible.push_back((float)totalSusceptible / (float)totalCells);
	graphInfectious.push_back((float)totalInfectious / (float)totalCells);
	graphResistant.push_back((float)totalResistant / (float)totalCells);
	graphCounter.push_back((float)counter);
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
				image->setPixel(i, j, sf::Color(colorWater[0] * 255.0f, colorWater[1] * 255.0f, colorWater[2] * 255.0f, 255));
			}
			else if (crntField[i][j].state == Susceptible)
			{
				image->setPixel(i, j, sf::Color(colorSusceptible[0] * 255.0f, colorSusceptible[1] * 255.0f, colorSusceptible[2] * 255.0f, 255));
			}
			else if (crntField[i][j].state == Infectious)
			{
				image->setPixel(i, j, sf::Color(colorInfectious[0] * 255.0f, colorInfectious[1] * 255.0f, colorInfectious[2] * 255.0f, 255));
			}
			else
			{
				image->setPixel(i, j, sf::Color(colorResistant[0] * 255.0f, colorResistant[1] * 255.0f, colorResistant[2] * 255.0f, 255));
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
	growthSusceptible = 0;
	growthInfectious = 0;
	growthResistant = 0;

	for (unsigned int i = 0; i < NUM_CELLS; i++)
	{
		for (unsigned int j = 0; j < NUM_CELLS; j++)
		{
			if (prevField[i][j].state == Susceptible && checkInfectious(i, j) && randf() < (0.8f / (1.0f + 1800.0f * std::exp(-15.0f * prevField[i][j].popDensity))) + 0.1f)
			{
				crntField[i][j].state = Infectious;
				totalInfectious++;
				growthInfectious++;
				totalSusceptible--;
				growthSusceptible--;
			}
			else if (prevField[i][j].state == Infectious)
			{
				if (prevField[i][j].time >= infectiousTime && randf())
				{
					crntField[i][j].state = Resistant;
					crntField[i][j].time = 0;
					totalInfectious--;
					growthInfectious--;
					totalResistant++;
					growthResistant++;
				}
				else
				{
					crntField[i][j].time++;
				}
			}
			else if (prevField[i][j].state == Resistant && prevField[i][j].popDensity != 0.0f)
			{
				if (prevField[i][j].time >= resistantTime && randf())
				{
					crntField[i][j].state = Susceptible;
					crntField[i][j].time = 0;
					totalResistant--;
					growthResistant--;
					totalSusceptible++;
					growthSusceptible++;
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
	if (!topography.loadFromFile(topographyFile))
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
	// Initialize ImPlot
	ImPlot::CreateContext();
	// Read the maps
	readMaps();
	// Select first map
	topographyFile = maps[0];
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
				if (event.key.code == sf::Keyboard::Space)
				{
					paused = !paused;
				}
				if (event.key.code == sf::Keyboard::R)
				{
					// Reset counter
					counter = 0;
					// Reset Proportion Graph
					totalCells = 0;
					totalSusceptible = 0;
					totalInfectious = 0;
					totalResistant = 0;
					graphSusceptible.clear(); graphSusceptible.resize(100000);
					graphInfectious.clear(); graphInfectious.resize(100000);
					graphResistant.clear(); graphResistant.resize(100000);
					graphCounter.clear(); graphCounter.resize(100000);
					// Reset Growth Graph
					graphGrowthSusceptible.clear(); graphGrowthSusceptible.resize(100000);
					graphGrowthInfectious.clear(); graphGrowthInfectious.resize(100000);
					graphGrowthResistant.clear(); graphGrowthResistant.resize(100000);
					// Initialize all textures
					initializeTextures();
				}
				if (event.key.code == sf::Keyboard::S)
				{
					std::string filepath =
						"screenshots/Step " + std::to_string(counter) +
						" ,initInf " + std::to_string(initialInfectious) +
						" ,infTime " + std::to_string(infectiousTime) +
						" ,resTime " + std::to_string(resistantTime) +
						" ,Seed " + std::to_string(seed) +
						".png";
					fieldImage.saveToFile(filepath);
				}
			}
			if (event.type == sf::Event::MouseWheelScrolled)
			{
				if (event.key.code == sf::Mouse::VerticalWheel)
				{
					if (event.mouseWheel.x > 0 && speed + 2 <= 60)
					{
						speed += 2;
					}
					else if (event.mouseWheel.x < 0 && speed - 2 >= 1)
					{
						speed -= 2;
					}
				}
			}
		}

		// Update ImGUI UI
		ImGui::SFML::Update(window, deltaClock.restart());





		// ImGUI UI
		ImGui::Begin("Settings");
		// Handles restarting
		if (ImGui::Button("Restart (R)"))
		{
			// Reset counter
			counter = 0;
			// Reset Proportion Graph
			totalCells = 0;
			totalSusceptible = 0;
			totalInfectious = 0;
			totalResistant = 0;
			graphSusceptible.clear(); graphSusceptible.resize(100000);
			graphInfectious.clear(); graphInfectious.resize(100000);
			graphResistant.clear(); graphResistant.resize(100000);
			graphCounter.clear(); graphCounter.resize(100000);
			// Reset Growth Graph
			graphGrowthSusceptible.clear(); graphGrowthSusceptible.resize(100000);
			graphGrowthInfectious.clear(); graphGrowthInfectious.resize(100000);
			graphGrowthResistant.clear(); graphGrowthResistant.resize(100000);
			// Initialize all textures
			initializeTextures();
		}
		// Handles pausing
		ImGui::Checkbox("Pause (SPACE)", &paused);
		// Controls Variables
		ImGui::Text("Ctrl + Left Click on variables to change manually");
		ImGui::SliderInt("Speed (Mouse Wheel)", &speed, 1, 60);
		ImGui::SliderFloat("Initial Infectious", &initialInfectious, 0.000001f, 1.0f, "%.6f");
		ImGui::SliderInt("Infectious Time", &infectiousTime, 0, 365);
		ImGui::SliderInt("Resistant Time", &resistantTime, 0, 365);
		ImGui::SliderInt("Seed", &seed, 0, 999999999);
		// Controls colors
		ImGui::ColorEdit3("Susceptible", colorSusceptible);
		ImGui::ColorEdit3("Infectious", colorInfectious);
		ImGui::ColorEdit3("Resistant", colorResistant);
		ImGui::ColorEdit3("Water", colorWater);
		// Handles screenshot saving
		if (ImGui::Button("Save Screenshot (S)"))
		{
			std::string filepath = 
				"screenshots/Step " + std::to_string(counter) + 
				" ,initInf " + std::to_string(initialInfectious) + 
				" ,infTime " + std::to_string(infectiousTime) + 
				" ,resTime " + std::to_string(resistantTime) + 
				" ,Seed " + std::to_string(seed) +
				".png";
			fieldImage.saveToFile(filepath);
		}
		ImGui::End();





		// List of maps to be used
		ImGui::Begin("Maps");
		if (ImGui::Button("Refresh Maps"))
		{
			// Read the maps
			readMaps();
		}
		ImGui::Text("Maps:");
		for (unsigned int i = 0; i < maps.size(); i++)
		{
			if (ImGui::Button(getName(maps[i]).c_str()))
			{
				topographyFile = maps[i];
				// Reset counter
				counter = 0;
				// Reset Proportion Graph
				totalCells = 0;
				totalSusceptible = 0;
				totalInfectious = 0;
				totalResistant = 0;
				graphSusceptible.clear(); graphSusceptible.resize(100000);
				graphInfectious.clear(); graphInfectious.resize(100000);
				graphResistant.clear(); graphResistant.resize(100000);
				graphCounter.clear(); graphCounter.resize(100000);
				// Reset Growth Graph
				graphGrowthSusceptible.clear(); graphGrowthSusceptible.resize(100000);
				graphGrowthInfectious.clear(); graphGrowthInfectious.resize(100000);
				graphGrowthResistant.clear(); graphGrowthResistant.resize(100000);
				// Initialize all textures
				initializeTextures();
			}
		}
		ImGui::End();





		// Update Proportion Graph
		graphSusceptible.push_back((float)totalSusceptible / (float)totalCells);
		graphInfectious.push_back((float)totalInfectious / (float)totalCells);
		graphResistant.push_back((float)totalResistant / (float)totalCells);
		graphCounter.push_back((float)counter);
		// Graph Window
		ImGui::Begin("Proportion Graph");
		// Draw Settings
		ImGui::Checkbox("Susceptible", &drawSusceptible);
		ImGui::SameLine();
		ImGui::Checkbox("Infectious", &drawInfectious);
		ImGui::SameLine();
		ImGui::Checkbox("Resistant", &drawResistant);
		ImGui::Checkbox("Legend", &legend);
		ImGui::Text("Right Click on graph for more options");
		// Take care of flags
		ImPlotFlags flagLegend = 0;
		if (!legend)
			flagLegend = ImPlotFlags_NoLegend;
		// Draw Graph
		ImPlot::BeginPlot("Plot", "step", "proportion", ImVec2(-1, 0), flagLegend, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_Lock);
		if (drawSusceptible)
		{
			// Draw Susceptible
			ImPlot::PushStyleColor(0, ImVec4(colorSusceptible[0], colorSusceptible[1], colorSusceptible[2], 1.0f));
			ImPlot::PlotLine("Susceptible", &graphCounter.front(), &graphSusceptible.front(), graphSusceptible.size());
		}
		if (drawInfectious)
		{
			// Draw Infectious
			ImPlot::PushStyleColor(0, ImVec4(colorInfectious[0], colorInfectious[1], colorInfectious[2], 1.0f));
			ImPlot::PlotLine("Infectious", &graphCounter.front(), &graphInfectious.front(), graphInfectious.size());
		}
		if (drawResistant)
		{
			// Draw Resistant
			ImPlot::PushStyleColor(0, ImVec4(colorResistant[0], colorResistant[1], colorResistant[2], 1.0f));
			ImPlot::PlotLine("Resistant", &graphCounter.front(), &graphResistant.front(), graphResistant.size());
		}
		ImPlot::EndPlot();
		ImGui::End();





		// Update Growth Graph
		graphGrowthSusceptible.push_back((float)growthSusceptible);
		graphGrowthInfectious.push_back((float)growthInfectious);
		graphGrowthResistant.push_back((float)growthResistant);
		// Graph Window
		ImGui::Begin("Growth Graph");
		// Draw Settings
		ImGui::Checkbox("Growth Susceptible", &drawGrowthSusceptible);
		ImGui::SameLine();
		ImGui::Checkbox("Growth Infectious", &drawGrowthInfectious);
		ImGui::SameLine();
		ImGui::Checkbox("Growth Resistant", &drawGrowthResistant);
		ImGui::Checkbox("Lock Y-axis", &lockY);
		ImGui::SameLine();
		ImGui::Checkbox("Legend", &legendGrowth);
		ImGui::Text("Right Click on graph for more options");
		// Take care of Flags
		ImPlotFlags flagLock = 0;
		if (lockY)
			flagLock = ImPlotAxisFlags_Lock;
		ImPlotFlags flagGrowthLegend = 0;
		if (!legendGrowth)
			flagGrowthLegend = ImPlotFlags_NoLegend;
		// Draw Graph
		ImPlot::BeginPlot("Growth Plot", "step", "proportion", ImVec2(-1, 0), flagGrowthLegend, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit | flagLock);
		if (drawGrowthSusceptible)
		{
			// Draw Growth Susceptible
			ImPlot::PushStyleColor(0, ImVec4(colorSusceptible[0], colorSusceptible[1], colorSusceptible[2], 1.0f));
			ImPlot::PlotLine("Growth Susceptible", &graphCounter.front(), &graphGrowthSusceptible.front(), graphGrowthSusceptible.size());
		}
		if (drawGrowthInfectious)
		{
			// Draw Growth Infectious
			ImPlot::PushStyleColor(0, ImVec4(colorInfectious[0], colorInfectious[1], colorInfectious[2], 1.0f));
			ImPlot::PlotLine("Growth Infectious", &graphCounter.front(), &graphGrowthInfectious.front(), graphGrowthInfectious.size());
		}
		if (drawGrowthResistant)
		{
			// Draw Growth Resistant
			ImPlot::PushStyleColor(0, ImVec4(colorResistant[0], colorResistant[1], colorResistant[2], 1.0f));
			ImPlot::PlotLine("Growth Resistant", &graphCounter.front(), &graphGrowthResistant.front(), graphGrowthResistant.size());
		}
		ImPlot::EndPlot();
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
			std::string counterString = "Step: " + std::to_string(counter);
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
	}

	// End ImPlot
	ImPlot::DestroyContext();
	// End ImGUI
	ImGui::SFML::Shutdown();

	return 0;
}