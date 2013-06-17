/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QV4SSA_P_H
#define QV4SSA_P_H

#include "qv4jsir_p.h"

QT_BEGIN_NAMESPACE
class QTextStream;

namespace QQmlJS {
namespace V4IR {

class LifeTimeInterval {
public:
    struct Range {
        int start;
        int end;

        Range(int start = Invalid, int end = Invalid)
            : start(start)
            , end(end)
        {}

        bool covers(int position) const { return start <= position && position <= end; }
    };
    typedef QList<Range> Ranges;

private:
    Temp _temp;
    Ranges _ranges;
    int _end;
    int _reg;
    unsigned _isFixedInterval : 1;
    unsigned _isSplitFromInterval : 1;

public:
    enum { Invalid = -1 };

    LifeTimeInterval()
        : _end(Invalid)
        , _reg(Invalid)
        , _isFixedInterval(0)
        , _isSplitFromInterval(0)
    {}

    bool isValid() const { return _end != Invalid; }

    void setTemp(const Temp &temp) { this->_temp = temp; }
    Temp temp() const { return _temp; }
    bool isFP() const { return _temp.type == V4IR::DoubleType; }

    void setFrom(Stmt *from);
    void addRange(int from, int to);
    Ranges ranges() const { return _ranges; }

    int start() const { return _ranges.first().start; }
    int end() const { return _end; }
    bool covers(int position) const
    { foreach (const Range &r, _ranges) if (r.covers(position)) return true; return false; }

    int firstPossibleUsePosition(bool isPhiTarget) const { return start() + (isSplitFromInterval() || isPhiTarget ? 0 : 1); }

    int reg() const { return _reg; }
    void setReg(int reg) { Q_ASSERT(!_isFixedInterval); _reg = reg; }

    bool isFixedInterval() const { return _isFixedInterval; }
    void setFixedInterval(bool isFixedInterval) { _isFixedInterval = isFixedInterval; }

    LifeTimeInterval split(int atPosition, int newStart);
    bool isSplitFromInterval() const { return _isSplitFromInterval; }
    void setSplitFromInterval(bool isSplitFromInterval) { _isSplitFromInterval = isSplitFromInterval; }

    void dump(QTextStream &out) const;
    static bool lessThan(const LifeTimeInterval &r1, const LifeTimeInterval &r2);
    static bool lessThanForTemp(const LifeTimeInterval &r1, const LifeTimeInterval &r2);
};

class Optimizer
{
public:
    struct SSADeconstructionMove
    {
        Stmt *phi;
        Expr *source;
        Temp *target;

        bool needsConversion() const
        { return target->type != source->type; }
    };

public:
    Optimizer(Function *function)
        : function(function)
        , inSSA(false)
    {}

    void run();
    void convertOutOfSSA();

    bool isInSSA() const
    { return inSSA; }

    QHash<BasicBlock *, BasicBlock *> loopStartEndBlocks() const { return startEndLoops; }

    QList<SSADeconstructionMove> ssaDeconstructionMoves(BasicBlock *basicBlock) const;

    QList<LifeTimeInterval> lifeRanges() const;

    static void showMeTheCode(Function *function);

private:
    Function *function;
    bool inSSA;
    QHash<BasicBlock *, BasicBlock *> startEndLoops;
};

} // V4IR namespace
} // QQmlJS namespace
QT_END_NAMESPACE

#endif // QV4SSA_P_H
