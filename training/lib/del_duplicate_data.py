import json


def remove_duplicates(json_data):
    x_tuples = [tuple(x) for x in json_data["X"]]

    unique_tuples = set()
    indexes_to_keep = []
    for i, x in enumerate(x_tuples):
        if x not in unique_tuples:
            unique_tuples.add(x)
            indexes_to_keep.append(i)

    json_data["X"] = [json_data["X"][i] for i in indexes_to_keep]
    json_data["Y"] = [json_data["Y"][i] for i in indexes_to_keep]

    return json_data


if __name__ == "__main__":
    filename = "large_playout_data.json"
    with open(filename, "r") as file:
        json_data = json.load(file)

    print("Data size:", len(json_data["X"]))

    result = remove_duplicates(json_data)

    with open(filename, "w") as file:
        json.dump(result, file)

    print("Data size X:", len(result["X"]))
    print("Data size Y:", len(result["Y"]))
