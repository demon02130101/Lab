#ifdef DEBUG_MODE

class sfLine;
class Viewer;

#ifndef VIEWER_H
#define VIEWER_H

#include "Lab3.h"
#include "Tile.h"
#include <SFML/Graphics.hpp>
#include <stdexcept>
#include <unordered_map>



class sfLine: public sf::Drawable
{
private:
	sf::Vector2f unitPerpendicular;
    sf::Vertex vertices[4];

public:
    sfLine(const sf::Vector2f& point1, const sf::Vector2f& point2);

    void draw(sf::RenderTarget &target, sf::RenderStates states) const;

	void setColor(const sf::Color& color);
	void setP1Color(const sf::Color& color);
	void setP2Color(const sf::Color& color);
	void setThickness(float thickness);

	sf::FloatRect getGlobalBounds() const;

};

class Viewer
{
public:
    static sf::Color getRandomColor(int normAve = 128, int rangeRandom = 256, int alpha = 255);

protected:
	bool needRedraw;

	// The monitor's resolution, use .width and .height to access
	sf::VideoMode monitorResolution;

	sf::FloatRect viewBounds;
	sf::Vector2f cursorPos;
	sf::Vector2f cursorPosWorld;

	sf::RenderStates renderStates;
	sf::Transform inverseTransform;

	sf::Font font;
	sf::Text text;

    bool hasUpdated;
    bool isDragging;

	double zoomAccum;
	sf::RenderWindow window;
	sf::View view;

	// ========== Following are custom members ==========
    Lab3* pMaster = nullptr;

	bool isSelecting, hasSelection;
	int selectionX1, selectionY1, selectionX2, selectionY2;

	bool isOlTileVissible;
    float thicknessOlTile;

	sf::Color colorBlock;
    sf::Color colorSpace;

protected:
	void update(); // Update some status

	void handleKeyPressed(sf::Keyboard::Key keyCode);
	void handleKeyReleased(sf::Keyboard::Key keyCode);
	void handleMouseButtonPressed(sf::Mouse::Button button);
	void handleMouseMoved(sf::Event::MouseMoveEvent mouseMoveEvent);
	void handleMouseButtonReleased(sf::Mouse::Button button);
	void handleMouseWheelScrolled(sf::Event::MouseWheelScrollEvent mouseWheelScrollEvent);
	void handleEvent(sf::Event event);

	// ========== Following are custom functions ==========
	void draw();

public:
    Viewer(std::string title, Lab3* pMaster);

    void run();

};
#endif // VIEWER_H
#endif // DEBUG_MODE
