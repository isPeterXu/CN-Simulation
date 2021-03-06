//
// Generated file, do not edit! Created by nedtool 5.4 from messages/addrs_message.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __ADDRS_MESSAGE_M_H
#define __ADDRS_MESSAGE_M_H

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0504
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
    #include "pow_message_m.h"
    #include <vector>
    typedef std::vector<int> AddressesVector;
// }}

/**
 * Class generated from <tt>messages/addrs_message.msg:25</tt> by nedtool.
 * <pre>
 * message AddrsMessage extends POWMessage
 * {
 *     AddressesVector addresses;
 * }
 * </pre>
 */
class AddrsMessage : public ::POWMessage
{
  protected:
    AddressesVector addresses;

  private:
    void copy(const AddrsMessage& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const AddrsMessage&);

  public:
    AddrsMessage(const char *name=nullptr, short kind=0);
    AddrsMessage(const AddrsMessage& other);
    virtual ~AddrsMessage();
    AddrsMessage& operator=(const AddrsMessage& other);
    virtual AddrsMessage *dup() const override {return new AddrsMessage(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual AddressesVector& getAddresses();
    virtual const AddressesVector& getAddresses() const {return const_cast<AddrsMessage*>(this)->getAddresses();}
    virtual void setAddresses(const AddressesVector& addresses);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const AddrsMessage& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, AddrsMessage& obj) {obj.parsimUnpack(b);}


#endif // ifndef __ADDRS_MESSAGE_M_H

