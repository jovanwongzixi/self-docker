# self-docker

## Description
A short C program to create a basic Linux container through isolating namespaces. This is a program developed as part of a three-part series of Medium articles about better understanding Docker Architecture. Links to these articles are provided below.

## Medium Articles
- [Deconstructing Docker Part I: Understanding Containers](https://medium.com/@zixiwong/deconstructing-docker-part-i-understanding-containers-87afacadcf)
- [Deconstructing Docker Part II: Understanding Docker Architecture](https://blog.devops.dev/deconstructing-docker-part-ii-understanding-docker-architecture-9cccf5d79981)
- [Deconstructing Docker Part III: Creating Our Own Container](https://blog.devops.dev/deconstructing-docker-part-iii-creating-our-own-container-6128b8fc7ba6)

## Download Project
```bash
git clone https://github.com/jovanwongzixi/self-docker.git
cd self-docker
```
## Project Setup
```bash
make
```
## Run Project
```bash
sudo ./self-docker /bin/bash
```
