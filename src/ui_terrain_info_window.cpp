#include <SFML/Graphics/Image.hpp>
#include "ui_terrain_info_window.h"

using namespace Archipelago;

UiTerrainInfoWindow::UiTerrainInfoWindow(ECS::World* world) :
	_world(world),
	_window(sfg::Window::Create(sfg::Window::BACKGROUND | sfg::Window::TITLEBAR)),
	_hTileSprite(sfg::Image::Create()),
	_hTileName(sfg::Label::Create("TileName")),
	_hProdTypeLabel(sfg::Label::Create("Unknown")),
	_hProdAmountLabel(sfg::Label::Create("0"))
{
	_world->subscribe<TerrainInfoWindowDataUpdateEvent>(this);

	_window->SetTitle("Terrain info");
	_window->SetRequisition({ 100.0f, 0.0f });
	_window->Show(false);

	_window->RemoveAll();

	sf::Image default_image;
	default_image.loadFromFile("assets/textures/default_map/plains.png");
	_hTileSprite->SetImage(default_image);

	auto rootLayoutBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 10.0f);
	
	auto tileInfoBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
	tileInfoBox->Pack(_hTileSprite, false);
	tileInfoBox->Pack(_hTileName, false);
	rootLayoutBox->Pack(tileInfoBox);
	
	_addBuildingInfoLayout(rootLayoutBox);

	_window->Add(rootLayoutBox);
}

UiTerrainInfoWindow::~UiTerrainInfoWindow() {
	_world->unsubscribe<TerrainInfoWindowDataUpdateEvent>(this);
}

void UiTerrainInfoWindow::receive(ECS::World* world, const TerrainInfoWindowDataUpdateEvent& event) {
	spdlog::get(loggerName)->trace("TerrainInfoWindowDataUpdateEvent received");
}

void UiTerrainInfoWindow::_addBuildingInfoLayout(sfg::Box::Ptr rootLayoutWidget) {
	auto buildingDescription = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
	auto descLabel = sfg::Label::Create("Base camp is the first building. It gives some initial resources. May be placed on flatlands tile");
	descLabel->SetLineWrap(true);
	descLabel->SetRequisition(sf::Vector2f(BUILDING_DESCRIPTION_LABEL_REQUISITION, 0.0f));
	descLabel->SetAlignment(sf::Vector2f(0.0f, 0.0f));
	buildingDescription->Pack(descLabel);
	rootLayoutWidget->Pack(buildingDescription);
	rootLayoutWidget->Pack(sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL));

	auto productionTypeHBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
	auto bpl = sfg::Label::Create("Building production:");
	bpl->SetAlignment(sf::Vector2f(0.0f, 0.0f));
	productionTypeHBox->Pack(bpl);
	_hProdTypeLabel->SetAlignment(sf::Vector2f(1.0f, 0.0f));
	productionTypeHBox->Pack(_hProdTypeLabel);
	rootLayoutWidget->Pack(productionTypeHBox);

	auto productionAmountHBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
	auto papm = sfg::Label::Create("Production amount per month:");
	papm->SetAlignment(sf::Vector2f(0.0f, 0.0f));
	productionAmountHBox->Pack(papm);
	_hProdAmountLabel->SetAlignment(sf::Vector2f(1.0f, 0.0f));
	productionAmountHBox->Pack(_hProdAmountLabel);
	rootLayoutWidget->Pack(productionAmountHBox);
}

void UiTerrainInfoWindow::_addTerrainInfoLayout(sfg::Box::Ptr rootLayoutWidget) {

}
