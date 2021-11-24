for project_name in inst malloc rbt
    do
        python3 ./cmd.py build $project_name
        python3 ./cmd.py run $project_name
    done 