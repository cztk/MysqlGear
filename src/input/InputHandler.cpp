//
// Created by ztk on 2021-07-17.
//

#include "InputHandler.h"

InputHandler::InputHandler(Config *pConfig, utils::LockedQueue<MysqlGearQueueEntry> *pQueue, LibzdbHandler *pDatabasehandler, ThreadpoolHandler *pDatabaseTasksThreadpool) :
config(pConfig), messages(pQueue), databasehandler(pDatabasehandler), databaseTasksThreadpool(pDatabaseTasksThreadpool) {
}

InputHandler::~InputHandler() = default;

void InputHandler::deinitialize() {

    if(nullptr != this->gearServer) {
        this->gearServer->stop();
        if(this->gearServerThread.joinable()) {
            this->gearServerThread.join();
        }
    }

}

void InputHandler::initialize() {

    if (config->gear_server_enable) {
        gearServer = new GearServer(config, messages, databasehandler, databaseTasksThreadpool);
        gearServer->initialize();
        gearServerThread = gearServer->run();
    }

}

