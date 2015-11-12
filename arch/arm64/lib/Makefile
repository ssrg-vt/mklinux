lib-y		:= bitops.o delay.o					\
		   strncpy_from_user.o strnlen_user.o clear_user.o	\
		   copy_from_user.o copy_to_user.o copy_in_user.o	\
		   copy_page.o clear_page.o				\
		   memchr.o memcpy.o memmove.o memset.o			\
		   strchr.o strrchr.o 
ifeq ($(CONFIG_CPU_BIG_ENDIAN), y)
else
lib-y		+= strcmp.o strncmp.o memcmp.o				\
		   strlen.o strnlen.o
endif
