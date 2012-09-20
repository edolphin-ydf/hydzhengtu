/*
 文件名 : Node.h
 创建时间 : 2012/9/18
 作者 : hyd
 功能 : 
*/
#ifndef __Node_H__
#define __Node_H__

/**
 * \brief 使所有的子类禁用拷贝构造函数和赋值符号
 *
 */
class CNode
{

  protected:

    /**
     * \brief 缺省构造函数
     *
     */
    CNode() {};

    /**
     * \brief 缺省析构函数
     *
     */
    ~CNode() {};

  private:

    /**
     * \brief 拷贝构造函数，没有实现，禁用掉了
     *
     */
    CNode(const CNode&);

    /**
     * \brief 赋值操作符号，没有实现，禁用掉了
     *
     */
    const CNode & operator= (const CNode &);

};

#endif

