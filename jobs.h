class cUnit;
class cGameTimer;

/**
* little helper jobs for game time synchonous actions,
* like rotating a unit to a spezific direction or landing/takeoff
*/
class cJob
{
protected:
	explicit cJob (cUnit* unit_);
public:
	bool finished;
	virtual void run (const cGameTimer& gameTimer) = 0;
	cUnit* unit;
};


class cStartBuildJob : public cJob
{
private:
	int orgX;
	int orgY;
	bool big;
public:
	cStartBuildJob (cUnit* unit_, int orgX_, int orgY_, bool big_);
	virtual void run (const cGameTimer& gameTimer);
};


class cPlaneTakeoffJob : public cJob
{
private:
	bool takeoff;
public:
	cPlaneTakeoffJob (cUnit* unit_, bool takeoff_);
	virtual void run (const cGameTimer& gameTimer);
};
