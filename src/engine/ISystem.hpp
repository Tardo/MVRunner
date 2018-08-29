/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_ENGINE_INTERFACE_SYSTEM
#define H_ENGINE_INTERFACE_SYSTEM


class ISystem
{
public:
	virtual ~ISystem() { }

	virtual bool init() = 0;
	virtual void reset() = 0;
	virtual void update(float deltaTime) = 0;
};

#endif
