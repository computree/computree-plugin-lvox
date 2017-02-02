#ifndef LVOX3_GENERICCOMPUTE_H
#define LVOX3_GENERICCOMPUTE_H

#include "mk/tools/lvox3_worker.h"

#include "ct_itemdrawable/abstract/ct_abstractgrid3d.h"
#include "mk/tools/lvox3_genericconfiguration.h"

#include "muParser.h"

/**
 * @brief Use a parser, a formula and multiple input grids to compute the output grid
 */
class LVOX3_GenericCompute : public LVOX3_Worker
{
    Q_OBJECT

public:
    struct Input {
        char gridLetterInFormula;
        CT_AbstractGrid3D* grid;
    };

    /**
     * @brief Create a worker that will use the formula to compute the output grid
     * @param inputs : list of input grid
     * @param formula : formula to use
     * @param output : output grid
     */
    LVOX3_GenericCompute(const QList<Input>& inputs,
                         const QList<lvox::CheckConfiguration>& checksFormula,
                         const std::string& finalFormula,
                         CT_AbstractGrid3D* output);
    ~LVOX3_GenericCompute();

protected:
    /**
     * @brief Do the job
     */
    void doTheJob();

private:

    struct InternalInput {
        int indexInVariables;
        CT_AbstractGrid3D* grid;
    };

    QVector<InternalInput>  m_inputs;
    QVector<double>         m_checksErrorCode;

    std::string             m_finalFormula;
    CT_AbstractGrid3D*      m_output;

    QVector<mu::Parser*>    m_checksParsers;
    mu::Parser              m_finalParser;

    QVector<double*>        m_variables;

    void initParser(mu::Parser& parser, const std::string &formula);
};

#endif // LVOX3_GENERICCOMPUTE_H
