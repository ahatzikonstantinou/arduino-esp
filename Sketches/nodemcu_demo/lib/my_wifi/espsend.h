#ifndef espsend_h
#define espsend_h

class EspSendClass
{
  public:
    virtual void State();
    virtual void Mode();
    virtual void Status();
    virtual void Name();

    virtual void Finish();
};

#endif
