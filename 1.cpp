#include <iostream>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <limits>
using namespace std;

class Simplex
{
public:
    enum type
    {
        LESS_THAN_EQ,
        GREATER_THAN_EQ,
        EQUAL_TO
    };

    struct constraint
    {
        vector<pair<float, string>> lhs;
        type type;
        float rhs;
    };

    void addVar(string var)
    {
        vars.push_back(var);
    }
    void addConstr(constraint c)
    {
        constrs.push_back(c);
    }
    void addObjective(vector<pair<float, string>> obj)
    {
        objective = obj;
    }
    void print()
    {
        cout << "Vars: ";
        for (auto v : vars)
            cout << v << " ";

        cout << endl;
        cout << "Constraints: " << endl;
        for (auto c : constrs)
        {
            cout << "(";
            bool f = 1;
            for (auto l : c.lhs)
                cout << (f ? "" : " + ") << l.first << "*" << l.second, f = 0;
            cout << ") "
                 << (c.type == LESS_THAN_EQ ? "<=" : (c.type == GREATER_THAN_EQ ? ">=" : "=="))
                 << " " << c.rhs << endl;
        }

        cout << "Objective: ";
        bool f = 1;
        for (auto o : objective)
            cout << (f ? "" : " + ") << o.first << "*" << o.second, f = 0;
        cout << endl;
        cout << endl;
    }

    void print_tabelau()
    {
        for (int i = 0; i < tabelau.size(); ++i)
        {
            for (int j = 0; j < tabelau[i].size(); ++j)
                cout << setw(2) << tabelau[i][j] << " ";
            cout << endl;
        }
    }

    void solve()
    {
        make_slack();
        make_tabelau();

        // print_tabelau();

        int last = -1;
        while ((last = pivot()) == 1)
            ;

        if (last == -1)
            cout << "No solution" << endl;
        else
        {
            cout << "Solution: " << endl;

            for (int i = 0; i < vars.size(); ++i)
            {
                int row = -1;
                if (is_basic(i, row))
                    cout << vars[i] << " = " << tabelau[row][tabelau[row].size() - 1] << endl;
                else
                    cout << vars[i] << " = 0" << endl;
            }
        }
    }

private:
    void make_slack()
    {
        int slack_cnt = 0;
        for (int i = 0; i < constrs.size(); ++i)
        {
            if (constrs[i].type == LESS_THAN_EQ)
            {
                vars.push_back("s" + to_string(slack_cnt));
                constrs[i].lhs.push_back(make_pair(1, "s" + to_string(slack_cnt++)));
                constrs[i].type = EQUAL_TO;
            }
            else if (constrs[i].type == GREATER_THAN_EQ)
            {
                vars.push_back("s" + to_string(slack_cnt));
                constrs[i].lhs.push_back(make_pair(-1, "s" + to_string(slack_cnt++)));
                constrs[i].type = EQUAL_TO;
            }
        }
    }
    void make_tabelau()
    {
        vector<vector<float>> _tabelau(constrs.size() + 1, vector<float>(vars.size() + 1));

        unordered_map<string, int> var_map;
        for (int i = 0; i < vars.size(); ++i)
            var_map[vars[i]] = i;

        for (int i = 0; i < objective.size(); ++i)
        {
            _tabelau[0][var_map[objective[i].second]] = -objective[i].first;
        }

        for (int i = 0; i < constrs.size(); ++i)
        {
            for (int j = 0; j < constrs[i].lhs.size(); ++j)
            {
                _tabelau[1 + i][var_map[constrs[i].lhs[j].second]] += constrs[i].lhs[j].first;
                _tabelau[1 + i][vars.size()] = constrs[i].rhs;
            }
        }

        tabelau = _tabelau;
    }

    int find_pivot_objective()
    {
        int pivot_col = -1;

        for (int j = 0; j < tabelau[0].size() - 1; ++j)
        {
            if (pivot_col == -1)
            {
                if (tabelau[0][j] < 0)
                    pivot_col = j;
            }
            else
            {
                if (tabelau[0][j] < 0 && tabelau[0][j] < tabelau[0][pivot_col])
                    pivot_col = j;
            }
        }

        return pivot_col;
    }
    int find_pivot_constraint(int col)
    {
        int pivot_row = -1;
        float pivot_mn = numeric_limits<float>::max();

        for (int i = 1; i < tabelau.size(); ++i)
        {
            if (tabelau[i][col] > 0)
            {
                float mn = tabelau[i][tabelau[i].size() - 1] / tabelau[i][col];
                if (mn < pivot_mn)
                {
                    pivot_mn = mn;
                    pivot_row = i;
                }
            }
        }

        return pivot_row;
    }

    int pivot()
    {
        int pivot_col = find_pivot_objective();
        if (pivot_col == -1)
            return 0;

        int pivot_row = find_pivot_constraint(pivot_col);

        cout << "Pivot: " << pivot_col << " " << pivot_row << endl;

        // print_tabelau();
        // cout << "->" << endl;

        float pivot_val = tabelau[pivot_row][pivot_col];
        for (int i = 0; i < tabelau[pivot_row].size(); ++i)
            if (abs(tabelau[pivot_row][i]) >= 0.0000001)
                tabelau[pivot_row][i] /= pivot_val;

        // print_tabelau();
        // cout << endl;

        for (int i = 0; i < tabelau.size(); ++i)
        {
            if (i == pivot_row)
                continue;

            // cout << tabelau[pivot_row][tabelau[pivot_row].size() - 1] << endl;

            int t = tabelau[i][pivot_col];
            for (int j = 0; j < tabelau[i].size(); ++j)
                tabelau[i][j] -= tabelau[pivot_row][j] * t;
        }

        // print_tabelau();
        // cout << endl;

        return 1;
    }

    bool is_basic(int col, int &row)
    {
        int cnt1 = 0, cnt0 = 0;
        for (int i = 0; i < tabelau.size(); ++i)
            if (abs(tabelau[i][col] - 1.) <= 0.0000001)
                ++cnt1, row = i;
            else if (abs(tabelau[i][col]) <= 0.0000001)
                ++cnt0;

        return cnt1 == 1 && cnt0 == tabelau.size() - 1;
    }

    vector<string> vars;
    vector<constraint> constrs;
    vector<pair<float, string>> objective;

    vector<vector<float>> tabelau;
};

int main()
{
    Simplex s;

    s.addVar("x");
    s.addVar("y");
    s.addVar("z");

    // s.addConstr({{{2, "x1"}, {1, "x2"}}, Simplex::LESS_THAN_EQ, 20});
    // s.addConstr({{{1, "x1"}, {1, "x2"}}, Simplex::LESS_THAN_EQ, 18});
    // s.addConstr({{{1, "x1"}}, Simplex::LESS_THAN_EQ, 8});

    // s.addObjective({{7, "x1"}, {4, "x2"}});

    s.addConstr({{{4, "x"}, {2, "y"}, {1, "z"}}, Simplex::LESS_THAN_EQ, 10});
    s.addConstr({{{2, "x"}, {5, "y"}, {3, "z"}}, Simplex::LESS_THAN_EQ, 15});

    s.addObjective({{-2, "x"}, {-3, "y"}, {-4, "z"}});

    s.print();
    s.solve();

    return 0;
}
