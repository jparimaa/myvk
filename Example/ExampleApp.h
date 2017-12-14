#pragma once

#include "../Framework/Application.h"

class ExampleApp : public fw::Application {
public:
    ExampleApp();
    virtual ~ExampleApp();
    ExampleApp(const ExampleApp&) = delete;
    ExampleApp(ExampleApp&&) = delete;
    ExampleApp& operator=(const ExampleApp&) = delete;
    ExampleApp& operator=(ExampleApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;
    virtual void render() final;

private:
    VkRenderPass renderPass = VK_NULL_HANDLE;
    
    bool createRenderPass();
    bool createPipeline();
};
