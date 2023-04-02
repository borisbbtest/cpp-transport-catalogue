#pragma once
#include <optional>
#include <unordered_set>
#include "json_reader.h"

namespace transport_catalog::RequestHandler
{

    class RequestHandler
    {
    public:
        // MapRenderer понадобится в следующей части итогового проекта
        RequestHandler(const TransportCatalogue &db, const svgreader::MapRenderer &renderer);

        // Возвращает информацию о маршруте (запрос Bus)
        // std::optional<BusStat> GetBusStat(const std::string_view &bus_name) const;

        // Возвращает маршруты, проходящие через
        //const std::unordered_set<BusPtr> *GetBusesByStop(const std::string_view &stop_name) const;

        // Этот метод будет нужен в следующей части итогового проекта
        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue &db_;
        const svgreader::MapRenderer &renderer_;
    };

}