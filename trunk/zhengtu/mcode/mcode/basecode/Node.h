/*
 �ļ��� : Node.h
 ����ʱ�� : 2012/9/18
 ���� : hyd
 ���� : 
*/
#ifndef __Node_H__
#define __Node_H__

/**
 * \brief ʹ���е�������ÿ������캯���͸�ֵ����
 *
 */
class CNode
{

  protected:

    /**
     * \brief ȱʡ���캯��
     *
     */
    CNode() {};

    /**
     * \brief ȱʡ��������
     *
     */
    ~CNode() {};

  private:

    /**
     * \brief �������캯����û��ʵ�֣����õ���
     *
     */
    CNode(const CNode&);

    /**
     * \brief ��ֵ�������ţ�û��ʵ�֣����õ���
     *
     */
    const CNode & operator= (const CNode &);

};

#endif

