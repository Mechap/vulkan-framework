#pragma once

#include <memory>

class Instance;
class Device;

class Renderer {
private:
	std::unique_ptr<Instance> instance{nullptr};
	std::unique_ptr<Device> device{nullptr};
};
