#pragma once

#include <iostream>

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QList>
#include <QVector>

class TestObject : public QObject {
    Q_OBJECT
public slots:
    void method( const QString& msg ) {
        std::cout << msg.toStdString() << std::endl;
    }
    void emitSignal( const QString& msg ) { std::cout << "emitting signal aSignal(" << msg.toStdString() << ")" << std::endl;
                                            emit aSignal( msg ); }
    void aSlot( const QString& msg ) { std::cout << "aSlot() called with data: " << msg.toStdString() << std::endl; }
    QString copyString( const QString& s ) { return s; }
    QVariantMap copyVariantMap( const QVariantMap& vm ) { return vm; }
    QVariantList copyVariantList( const QVariantList& vl ) { return vl; }
    QObject* createObject() { 
        TestObject* mo = new TestObject; 
        mo->setObjectName( "New Object" );
        return mo; //WARNING: NOT DESTROYED WHEN GARBAGE COLLECTED 
                   //SINCE DEFAULT IS 'QOBJ_NO_DELETE'
    }
    QList< float > copyFloatList( const QList< float >& l ) { return l; }
    QVector< float > copyFloatVector( const QVector< float >& v ) { return v; }
    QList< short > copyShortList( const QList< short >& l ) { return l; }
    QVector< short > copyShortVector( const QVector< short >& v ) { return v; }
signals:
    void aSignal(const QString&);
};
