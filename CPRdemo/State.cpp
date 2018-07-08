#include <Arduino.h>

class State {
  public:
    State::State(int id) {
      Serial.println("Initializing state with id " + (String)id);
      _id = id;
    }
    virtual void Init() {
      Serial.println("Initializing base state");
    }
    virtual void DeInit() {
      
    }
    virtual void Update() {
      Serial.println("Updating base state");
    }
    int GetId() {
      Serial.println("My id is " + (String)_id);
      return _id;
    }
  protected:
    int _id;
};

class SetupState : public State {
  
  public:
    SetupState(int id, String message) : State(id) {
      Serial.println( message);
      }
    void Init() {
      Serial.println("Initializing setup state");
    }

    void DeInit() {
      Serial.println("Deinitializing setup state");
    }

    void Update()
    {
      Serial.println("Updating setup state");
    }
};

class PlayState : public State {
  public:
    PlayState(int id) : State(id) {}
    void Init() {
      Serial.println("Initializing play state");
    }

    void DeInit() {
      Serial.println("Deinitializing play state");
    }

    void Update()
    {
      Serial.println("Updating play state");
    }
};

class FeedbackState : public State {
  public:
    FeedbackState(int id) : State(id) {}
    
    void Init() {
      Serial.println("Initializing feedback state");
    }

    void DeInit() {
      Serial.println("Deinitializing feedback state");
    }

    void Update()
    {
      Serial.println("Updating feedback state");
    }
};

class CalibrationState : public State {
  public:
    CalibrationState(int id) : State(id) {}
    
    void Init() {
      Serial.println("Initializing calibration state");
    }

    void DeInit() {
      Serial.println("Deinitializing calibration state");
    }

    void Update()
    {
      Serial.println("Updating calibration state");
    }
};


