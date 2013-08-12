#ifndef __COMMON_TQUEUE_H__
#define __COMMON_TQUEUE_H__

template <class Type>
class TQueue
{
public:
    TQueue( );
    virtual ~TQueue( );

public:
    virtual void push(Type);
    virtual Int  purge(Type);
    virtual Type* next(Type);
    
private:
    Type*     m_first;
};



template <class Type> TQueue<Type>::TQueue( ) :
    m_first(0)
{
    COMMON("(+) TQueue");
}


template <class Type> TQueue<Type>::~TQueue( )
{
    COMMON("(-) TQueue");

}




#endif /* __COMMON_TQUEUE_H__ */