#ifdef DEBUG_MODE

#include "Viewer.h"
#include "Lab3.h"
#include <math.h>

using namespace std;



sfLine::sfLine(const sf::Vector2f& point1, const sf::Vector2f& point2)
{
    sf::Vector2f direction = point2 - point1;
    sf::Vector2f unitDirection = 
        direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);
    this->unitPerpendicular = sf::Vector2f(-unitDirection.y, unitDirection.x);

    // the default thickness is 1.f
    sf::Vector2f offset = (1.f / 2.f) * unitPerpendicular;
    vertices[0].position = point1 + offset;
    vertices[1].position = point2 + offset;
    vertices[2].position = point2 - offset;
    vertices[3].position = point1 - offset;

    // the default color is black
    for (int i = 0; i < 4; ++i) { vertices[i].color = sf::Color::Black; }
}

void sfLine::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(vertices, 4, sf::Quads, states);
}

void sfLine::setColor(const sf::Color& color)
{
    for (int i = 0; i < 4; ++i) { vertices[i].color = color; }
}
void sfLine::setP1Color(const sf::Color& color)
{
    vertices[0].color = color; vertices[3].color = color;
}
void sfLine::setP2Color(const sf::Color& color)
{
    vertices[1].color = color; vertices[2].color = color;
}
void sfLine::setThickness(float thickness)
{
    sf::Vector2f
        point1 = sf::Vector2f(
            (vertices[0].position.x + vertices[3].position.x) / 2,
            (vertices[0].position.y + vertices[3].position.y) / 2),
        point2 = sf::Vector2f(
            (vertices[1].position.x + vertices[2].position.x) / 2,
            (vertices[1].position.y + vertices[2].position.y) / 2);

    sf::Vector2f offset = (thickness / 2.f) * unitPerpendicular;
    vertices[0].position = point1 + offset;
    vertices[1].position = point2 + offset;
    vertices[2].position = point2 - offset;
    vertices[3].position = point1 - offset;
}

sf::FloatRect sfLine::getGlobalBounds() const
{
    // Calculate the bounding box of the transformed vertices
    float minX = std::min({vertices[0].position.x, vertices[1].position.x, vertices[2].position.x, vertices[3].position.x});
    float minY = std::min({vertices[0].position.y, vertices[1].position.y, vertices[2].position.y, vertices[3].position.y});
    float maxX = std::max({vertices[0].position.x, vertices[1].position.x, vertices[2].position.x, vertices[3].position.x});
    float maxY = std::max({vertices[0].position.y, vertices[1].position.y, vertices[2].position.y, vertices[3].position.y});

    return sf::FloatRect(minX, minY, maxX - minX, maxY - minY);
}



sf::Color Viewer::getRandomColor(int normAve, int rangeRandom, int alpha)
{
    int halfRange = rangeRandom / 2;

    return sf::Color(
        (normAve + std::rand() % rangeRandom - halfRange) % 256,
        (normAve + std::rand() % rangeRandom - halfRange) % 256,
        (normAve + std::rand() % rangeRandom - halfRange) % 256,
        alpha
    );
}

Viewer::Viewer(string title, Lab3* pMaster):
    needRedraw(true),
    monitorResolution(sf::VideoMode::getDesktopMode()),
    viewBounds(0, 0, 0, 0),
    cursorPos(-1, -1), cursorPosWorld(-1, -1),
    renderStates(sf::RenderStates::Default), inverseTransform(sf::Transform::Identity),
    hasUpdated(false), isDragging(false),
    window(sf::VideoMode(monitorResolution.width / 2, monitorResolution.height / 2), title),
    view(this->window.getDefaultView()),
    pMaster(pMaster),
    isSelecting(false), hasSelection(false),
    selectionX1(-1), selectionY1(-1), selectionX2(-1), selectionY2(-1),
    isOlTileVissible(true), thicknessOlTile(-2),
    colorBlock(255, 150, 150, 128), colorSpace(150, 255, 150, 128)
{
    std::srand(0);

    font.loadFromFile("/home/leo/Code/Fonts/Roboto-Regular.ttf");
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(2);
    text.setPosition(10, window.getSize().y - 10);
    text.setScale(1, -1);

    renderStates.transform.scale(1, -1).translate(0, -1.0 * window.getSize().y);
    inverseTransform.translate(0, window.getSize().y).scale(1, -1);

    window.setPosition(sf::Vector2i(monitorResolution.width - window.getSize().x, 0));
    window.setFramerateLimit(45);

    // Set a hard cap
    // so the scale won't be too large relative to the size of the instances
    this->zoomAccum = std::min(
        50.f,
        std::max(
            static_cast<float>(pMaster->width) / window.getSize().x,
            static_cast<float>(pMaster->height) / window.getSize().y
        )
    );

    thicknessOlTile *= this->zoomAccum;
    view.setSize(
        this->window.getDefaultView().getSize().x * this->zoomAccum,
        this->window.getDefaultView().getSize().y * this->zoomAccum);
    view.setCenter(view.getSize().x / 2, window.getSize().y - view.getSize().y / 2);
    window.setView(view);
}

void Viewer::update()
{
    if (hasUpdated) { return; }
    hasUpdated = true;

    this->cursorPosWorld = this->inverseTransform.transformPoint(
        this->window.mapPixelToCoords(
            sf::Mouse::getPosition(window),
            this->view
        )
    );

    this->viewBounds = sf::FloatRect(
        this->view.getCenter().x - this->view.getSize().x / 2,
        window.getSize().y - (this->view.getCenter().y + this->view.getSize().y / 2),
        this->view.getSize().x,
        this->view.getSize().y);

    // Since two adjacent tiles would make their shared edge twice as thick.
    static const float thresholdVisible = 0.5;
    this->isOlTileVissible = std::abs(this->thicknessOlTile / this->zoomAccum) >= thresholdVisible;

    return;
}

void Viewer::handleKeyPressed(sf::Keyboard::Key keyCode)
{
    static const float factorZoom = 0.1;

    sf::Vector2f cursorToViewCenter = this->cursorPosWorld - this->inverseTransform.transformPoint(view.getCenter());

    switch (keyCode)
    {
        case sf::Keyboard::Escape:
            window.close();
            break;

        case sf::Keyboard::Z:
            this->zoomAccum *= (1 - factorZoom);
            view.zoom(1 - factorZoom);
            view.move(factorZoom * cursorToViewCenter.x, -factorZoom * cursorToViewCenter.y);
            window.setView(view);
            break;
        
        case sf::Keyboard::X:
            this->zoomAccum *= (1 + factorZoom);
            view.zoom(1 + factorZoom);
            view.move(-factorZoom * cursorToViewCenter.x, factorZoom * cursorToViewCenter.y);
            window.setView(view);
            break;

        case sf::Keyboard::W:
            view.move(0, -view.getSize().y / 20);
            window.setView(view);
            break;
        
        case sf::Keyboard::A:
            view.move(-view.getSize().x / 20, 0);
            window.setView(view);
            break;
        
        case sf::Keyboard::S:
            view.move(0, view.getSize().y / 20);
            window.setView(view);
            break;
        
        case sf::Keyboard::D:
            view.move(view.getSize().x / 20, 0);
            window.setView(view);
            break;

        case sf::Keyboard::LAlt:
            this->isDragging = true;
            this->cursorPos = sf::Vector2f(sf::Mouse::getPosition(window));
            break;

        default:
            break;
    }
}
void Viewer::handleKeyReleased(sf::Keyboard::Key keyCode)
{
    switch (keyCode)
    {
        case sf::Keyboard::LAlt:
            this->isDragging = false;
            break;

        default:
            break;
    }
}
void Viewer::handleMouseButtonPressed(sf::Mouse::Button button)
{
    switch (button)
    {
        case sf::Mouse::Left:
            if (this->hasSelection)
            {
                this->hasSelection = false;
                selectionX1 = selectionY1 = selectionX2 = selectionY2 = -1;
            }
            else
            {
                this->isSelecting = true;
                this->hasSelection = true;
                selectionX1 = selectionX2 = this->cursorPosWorld.x;
                selectionY1 = selectionY2 = this->cursorPosWorld.y;
            }
            break;

        case sf::Mouse::Middle:
            this->isDragging = true;
            this->cursorPos = sf::Vector2f(sf::Mouse::getPosition(window));
            break;
        
        default:
            break;
    }
}
void Viewer::handleMouseMoved(sf::Event::MouseMoveEvent mouseMoveEvent)
{
    this->update();

    if (this->isSelecting)
    {
        this->selectionX2 = this->cursorPosWorld.x;
        this->selectionY2 = this->cursorPosWorld.y;
    }
    if (this->isDragging)
    {
        sf::Vector2f cursorPosNew = sf::Vector2f(mouseMoveEvent.x, mouseMoveEvent.y);
        sf::Vector2f offset = this->cursorPos - cursorPosNew;

        offset.x *= this->zoomAccum;
        offset.y *= this->zoomAccum;

        view.move(offset);
        window.setView(view);
        this->cursorPos = cursorPosNew;
    }
}
void Viewer::handleMouseButtonReleased(sf::Mouse::Button button)
{
    switch(button)
    {
        case sf::Mouse::Left:
            this->isSelecting = false;
            break;

        case sf::Mouse::Middle:
            this->isDragging = false;
            break;

        default:
            break;
    }
}
void Viewer::handleMouseWheelScrolled(sf::Event::MouseWheelScrollEvent mouseWheelScrollEvent)
{
    if (mouseWheelScrollEvent.delta > 0) { this->handleKeyPressed(sf::Keyboard::Z); }
    else { this->handleKeyPressed(sf::Keyboard::X); }
}
void Viewer::handleEvent(sf::Event event)
{
    this->needRedraw = true;

    switch (event.type)
    {
        case sf::Event::Closed:
            window.close();
            break;

        case sf::Event::KeyPressed:
            this->handleKeyPressed(event.key.code);
            break;

        case sf::Event::KeyReleased:
            this->handleKeyReleased(event.key.code);
            break;

        case sf::Event::MouseButtonPressed:
            this->handleMouseButtonPressed(event.mouseButton.button);
            break;

        case sf::Event::MouseMoved:
            this->handleMouseMoved(event.mouseMove);
            break;

        case sf::Event::MouseButtonReleased:
            this->handleMouseButtonReleased(event.mouseButton.button);
            break;

        case sf::Event::MouseWheelScrolled:
            this->handleMouseWheelScrolled(event.mouseWheelScroll);
            break;

        default:
            break;
    }
}

void Viewer::draw()
{
    string strToShow =
        std::to_string(static_cast<int>(this->cursorPosWorld.x)) + ", " +
        std::to_string(static_cast<int>(this->cursorPosWorld.y)) + '\n';

    // Draw the background
    sf::RectangleShape whiteBackGround(sf::Vector2f(this->pMaster->width, this->pMaster->height));
    whiteBackGround.setPosition(this->pMaster->x, this->pMaster->y);
    if (this->viewBounds.intersects(whiteBackGround.getGlobalBounds()))
    {
        whiteBackGround.setFillColor(sf::Color::White);
        this->window.draw(whiteBackGround, this->renderStates);
    }

    // Draw the tiles
    Tile regionView = Tile(
        "View",
        this->viewBounds.left,
        this->viewBounds.top,
        this->viewBounds.width,
        this->viewBounds.height
    );
    for (Tile* pTile: this->pMaster->getSpTileInRegion(regionView))
    {
        sf::RectangleShape rect(sf::Vector2f(pTile->width, pTile->height));
        rect.setPosition(pTile->left(), pTile->bottom());

        rect.setFillColor(pTile->isBlock()? this->colorBlock: this->colorSpace);
        if (this->isOlTileVissible)
        {
            rect.setOutlineThickness(this->thicknessOlTile);
            rect.setOutlineColor(sf::Color::Black);
        }
        this->window.draw(rect, this->renderStates);
    }

    if (this->hasSelection)
    {
        Tile regionSelection(
            "Selection",
            std::min(selectionX1, selectionX2),
            std::min(selectionY1, selectionY2),
            std::abs(selectionX1 - selectionX2),
            std::abs(selectionY1 - selectionY2)
        );
        for (Tile* pTile: this->pMaster->getSpTileInRegion(regionSelection))
        {
            sf::RectangleShape rect(sf::Vector2f(pTile->width, pTile->height));
            rect.setPosition(pTile->left(), pTile->bottom());

            rect.setFillColor(pTile->isBlock()? this->colorBlock: this->colorSpace);
            if (this->isOlTileVissible)
            {
                rect.setOutlineThickness(this->thicknessOlTile);
                rect.setOutlineColor(sf::Color::Black);
            }
            this->window.draw(rect, this->renderStates);
        }

        sf::RectangleShape rect(sf::Vector2f(regionSelection.width, regionSelection.height));
        rect.setPosition(regionSelection.left(), regionSelection.bottom());
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineThickness(this->thicknessOlTile);
        rect.setOutlineColor(sf::Color::Black);
        this->window.draw(rect, this->renderStates);
    }

    // Draw text
    this->window.setView(this->window.getDefaultView());
    this->text.setString(strToShow);
    this->window.draw(text, this->renderStates);
    this->window.setView(this->view);
}

void Viewer::run()
{
    while (window.isOpen())
    {
        this->hasUpdated = false;

        sf::Event event;
        while (window.pollEvent(event)) { this->handleEvent(event); }

        if (this->needRedraw || this->isDragging)
        {
            this->update();
            window.clear();
            this->draw();
            window.display();
            this->needRedraw = false;
        }
    }
}

#endif // DEBUG_MODE
