# API - Search the File System

# Searching Files based on Name, Extension and Size

# input for the search (directory)

from abc import ABC, abstractmethod


class File:
    def __init__(self, name, type, size):
        self.name = name
        self.extension = type
        self.size = size

    def get_name(self):
        return self.name

    def get_extension(self):
        return self.extension

    def get_size(self):
        return self.size


class Filter(ABC):
    def __init__(self):
        pass

    @abstractmethod
    def match(self, file):
        pass


class NameFilter(Filter):
    def __init__(self, name):
        self.name = name

    def match(self, file):
        return file.get_name() == self.name


class SizeFilter(Filter):
    def __init__(self, properties):
        self.size = properties[0]
        self.operator = properties[1]

    # def match(self, file):
    #     return eval(str(file.get_size()) + str(self.operator) + str(self.size))

    def match(self, file):
        file_size = file.get_size()

        if self.operator == ">=":
            return file_size >= self.size
        if self.operator == ">":
            return file_size > self.size
        if self.operator == "<=":
            return file_size <= self.size
        if self.operator == "<":
            return file_size < self.size
        if self.operator == "==":
            return file_size == self.size

        return False


class ExtensionFilter(Filter):
    def __init__(self, extension):
        self.extension = extension

    def match(self, file):
        return file.get_extension() == self.extension


class FileSystem:
    def __init__(self, name, isDirectory=False, subDirectories=[], files=[]):
        self.name = name
        self.isDirectory = isDirectory
        self.subDirectory = subDirectories
        self.files = files

    def add_file(self, directory, sub_directory, file):
        pass

    def delete_file(self, directory, sub_directory, file):
        pass


class Search:
    def __init__(self, directory, filter, fileSystem, condition=None):
        self.directory = directory
        self.filters = filter
        self.fileSystem = fileSystem
        self.condition = condition

    def check_conditions(self, file, instances, condition):
        if condition is None:
            return instances[0].match(file)
        elif condition == "AND":
            return all([instance.match(file) for instance in instances])
        else:
            return any([instance.match(file) for instance in instances])


    def find_files(self):
        root = self.fileSystem
        queue = [root]

        filter_classes = []
        for filter, value in self.filters.items():
            filter_classes.append(globals().get(filter)(value))
        res = []

        # BFS
        while queue:
            for each_file in root.files:
                if self.check_conditions(each_file, filter_classes, each_file):
                    res.append(each_file.get_name())
            node = queue.pop(0)
            for each_subdirectory in node.subDirectory:
                queue.append(each_subdirectory)

        return res


if __name__ == "__main__":
    # Simulation
    f1 = File("abc", "txt", 10)
    f2 = File("cde", "txt", 20)
    f3 = File("def", "pdf", 30)
    f4 = File("ghi", "py", 5)
    f5 = File("uvw", "java", 10)

    directory_files = [f1, f2, f3, f4, f5]
    fileSystem = FileSystem("/", True, [], directory_files)

    #res = Search(directory_files, {"NameFilter": "abc"}, fileSystem).find_files()
    #print(res)

    #res = Search(directory_files, {"SizeFilter": (10, ">=")}, fileSystem).find_files()
    #print(res)

    res = Search(directory_files, {"ExtensionFilter": "java", "SizeFilter": (10, ">=")}, fileSystem, "OR").find_files()
    print(res)