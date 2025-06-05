#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>
#include "libft/libft.h"

int error(char *filename)
{
	write(2, filename, ft_strlen(filename));
	write(2, " ", 1);
	perror("open error\n");
	exit(1);
}

int main(int ac, char ** av)
{
    if (ac != 2) {
        write(2, "need only 2 args\n", 18);
        return(1);
    }
    int fd = open(av[1], O_RDONLY);
    if (fd < 0) {
		error(av[1]);
		return(1);
    }
    unsigned char magic[4];
    if (read(fd, magic, 4) != 4) {
        write(2, "read error\n", 11);
        close(fd);
        return 1;
    }
    close(fd);
    fd = open(av[1], O_RDONLY);
    if (fd < 0) {
		error(av[1]);
		return(1);
    }
    if (!(magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')) {
        write(2, "Not an ELF file\n", 16);
        close(fd);
        return 1;
    }
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat error");
        close(fd);
        return 1;
    }

	if (!S_ISREG(st.st_mode))
	{
		write (2, "Error : not a regular file\n", 27);
		close(fd);
		return(1);
	}
    size_t filesize = st.st_size;
    void *file_mem = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_mem == MAP_FAILED) {
        perror("mmap error");
        close(fd);
        return 1;
    }
    // TODO: Parcourir la table des symboles ELF et les afficher
    // Structures ELF de base
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file_mem;
    Elf64_Shdr *shdr = (Elf64_Shdr *)((char *)file_mem + ehdr->e_shoff);
    const char *shstrtab = (char *)file_mem + shdr[ehdr->e_shstrndx].sh_offset;
    Elf64_Shdr *symtab = NULL;
    Elf64_Shdr *strtab = NULL;
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            symtab = &shdr[i];
        } else if (shdr[i].sh_type == SHT_STRTAB && ft_strcmp_const(shstrtab + shdr[i].sh_name, ".strtab") == 0) {
            strtab = &shdr[i];
        }
    }
    if (!symtab || !strtab) {
		write(2, "ft_nm : ", 8);
		write(2, av[1], ft_strlen(av[1]));
        write(2, ": no symbols\n", 13);
    } else {
        Elf64_Sym *sym = (Elf64_Sym *)((char *)file_mem + symtab->sh_offset);
        int nsyms = symtab->sh_size / symtab->sh_entsize;
        const char *strtab_p = (char *)file_mem + strtab->sh_offset;
        for (int i = 0; i < nsyms; i++) {
            if (sym[i].st_name != 0 && (ELF64_ST_TYPE(sym[i].st_info) != STT_FILE)) {
                write(1, strtab_p + sym[i].st_name, ft_strlen(strtab_p + sym[i].st_name));
                write(1, "\n", 1);
            }
        }
    }
    munmap(file_mem, filesize);
    close(fd);
    return 0;
}
