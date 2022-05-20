import numpy as np
import json
import os
import pandas as pd
import re
import matplotlib as mpl
from matplotlib import pyplot as plt
from pandas.io.json import json_normalize


# Load instance json
def generateGantt(inst_name, sol_path, ins_path):
    sol_dir = os.path.join(sol_path, inst_name+".json")
    ins_dir = os.path.join(ins_path, "instances.json")
    with open(ins_dir) as data:
        ins_data = json.load(data)

    ins_data_f = json_normalize(ins_data)
    ins_name_lst = list(ins_data_f["name"])
    ins_path_list = list(ins_data_f["path"])

    name_idx = ins_name_lst.index(inst_name)
    name_path = os.path.join(ins_path, ins_path_list[name_idx])

    # Parse instance production time matrix P[i][j]
    with open(name_path) as data:
        data_lines = data.readlines()
        job_idx = 0
        for line in data_lines:
            if line.startswith('#'):
                continue
            line_split = line.strip('\n').split()
            if len(line_split) == 2:
                m = int(line_split[1])
                n = int(line_split[0])
                p_mat = np.zeros((m, n))
            elif len(line_split) > 2:
                for i in range(m):
                    machine_id = int(line_split[2*i])
                    proc_time = int(line_split[2*i+1])
                    p_mat[machine_id][job_idx] = proc_time
                job_idx += 1

    # Load solve json
    with open(sol_dir) as data:
        sol_data = json.load(data)

    sol_data_f = json_normalize(sol_data["Vars"])
    schedule = pd.DataFrame(sol_data_f)
    var_list = list(schedule["VarName"])
    var_value = list(schedule["X"])

    # Parse schedule matrix
    makespan = 0
    schedule_mat = np.zeros((m, n))
    x_pattern = r'x_{([\d]+), ([\d]+)}'

    for i in range(len(var_list)):
        var_name = var_list[i]
        if var_name.startswith("x"):
            ls = re.findall(x_pattern, var_name)
            schedule_mat[int(ls[0][0]), int(ls[0][1])] = int(var_value[i])
        if var_name.startswith("C"):
            makespan = var_value[i]

    # Set Gantt chart style
    bar_style = {'alpha':1.0, 'lw':12, 'solid_capstyle':'butt'}
    text_style = {'color':'white', 'weight':'bold', 'ha':'center', 'va':'center'}
    colors = mpl.cm.Dark2.colors

    # Generate the gantt chart in the horizon of job
    # Reference: https://jckantor.github.io/ND-Pyomo-Cookbook/04.03-Job-Shop-Scheduling.html
    fig, ax = plt.subplots(2,1, figsize=(10, (5+n+m)/4))
    for i in range(n):
        job_start_times = schedule_mat[:,i]
        job_end_times = p_mat[:,i] + job_start_times
        for j in range(m):
            job_start_time = job_start_times[j]
            job_end_time = job_end_times[j]
            ax[0].plot([job_start_time, job_end_time], [i]*2, c=colors[j%7], **bar_style)
            ax[0].text((job_start_time+job_end_time)/2, i, "M%d"%j, **text_style)
            ax[1].plot([job_start_time, job_end_time], [j]*2, c=colors[i%7], **bar_style)
            ax[1].text((job_start_time + job_end_time)/2, j, "J%d"%i, **text_style)

    yticks = ["J", "M"]
    for i, s in enumerate([n, m]):
        ax[i].set_ylim(-0.5, s-0.5)
        ax[i].set_yticks(range(0, s))
        ax[i].set_yticklabels([yticks[i]+str(x) for x in range(s)])
        ax[i].text(makespan, ax[i].get_ylim()[0]-0.2, "{0:0.1f}".format(makespan), ha='center', va='top')
        ax[i].plot([makespan]*2, ax[i].get_ylim(), 'r--')
        ax[i].set_xlabel('Time')
        ax[i].grid(True)

    ax[0].set_title('Job Schedule')
    ax[0].set_ylabel('Job')
    ax[1].set_title('Machine Schedule')
    ax[1].set_ylabel('Machine')
    fig.tight_layout()
    plt.show()

if __name__ == "__main__":
    inst_names = ["ft06", "la01", "ft10", "la06"]
    sol_path = r"C:/Users/paulz/source/repos/ProductionManagement/JSP_Gurobi"
    ins_path = r"F:/JSPLIB-master/"
    
    for inst_name in inst_names:
        generateGantt(inst_name, sol_path, ins_path)