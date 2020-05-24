#pragma once



#define SCHEDULER_ENABLED


// Тип результата выполненной операции

typedef int		FW_RESULT_TYPE;

#define	FW_OK						0
#define	FW_FAIL						-1
#define	FW_INSUFFICIENT_SPACE		-2
#define	FW_UNKNOWN_LOCAL_STORAGE	-3
#define	FW_UNMOUNTED_LOCAL_STORAGE	-4
#define	FW_FILE_WRITE_ERROR			-5
#define	FW_DISABLED_STREAM			-6
#define	FW_UNSUPPORTED_FILE_FORMAT	-7


// Тип манипулятора

typedef void*	FW_HANDLE_TYPE;


// Формат записываемого файла

typedef enum _FileWritingFormat
{
	kAviFileWritingFormat = 1,
	kJpgFileWritingFormat = 2
} FileWritingFormat;


// Предназначение записываемого файла

typedef enum _FileWritingDest
{
	kLocalStorageFileWritingDest = 0x1,
	kFtpFileWritingDest = 0x2,
	kEmailFileWritingDest = 0x4
} FileWritingDest;


// Функция обратного вызова. Вызывается из записывающего файл потока после окончания записи файла.
//  hFileWritingId - идентификатор записываемого файла.
//  hUserData - пользовательские данные, переданные в функцию StartFileWriting.
//  nResult - результат операции по записи файла.
//  lpFileName - полный путь к записанному файлу. Если файл записать не удалось, то равен NULL.
//		Удаление записанного файла осуществляется пользователем библиотеки.
//		Буфер с путем к файлу принадлежит библиотеке и не должен использоваться после 
//		выхода из функции обратного вызова.
//  nDuration - полная длительность записанного файла в мс.
//  return value - длительность в мс для следующего записываемого файла. Или 0, если следующий 
//		файл записывать не нужно.

typedef int (*FW_CALLBACK_TYPE)(FW_HANDLE_TYPE hFileWritingId,
								FW_HANDLE_TYPE hUserData,
								FW_RESULT_TYPE nResult,
								const char * lpFileName,
								int nDuration);


// Функция для инициализации библиотеки. Должна быть вызвана перед её использованием.
//  return value - 0 при успешной инициализации библиотеки. Ненулевое значение говорит о том, 
//		что библиотеку инициализировать не удалось и её дальнейшее использование невозможно.

FW_RESULT_TYPE InitializeFileWritingLibrary();


// Функция для деинициализации библиотеки. Должна быть вызвана после её использования.

FW_RESULT_TYPE UninitializeFileWritingLibrary();


// Функция записи файла.
//  fwDest - предназначение записываемого файла.
//  nStream - номер потока на камере для записи файла. Нумерация потоков начинается с 0.
//  fwFormat - формат записываемого файла.
//  nDurationBefore - интервал времени в мс, на которое желательно откатиться назад 
//		с текущего момента времени, и уже с него начать запись файла.
//  nDurationAfter - длительность записи видео в мс начиная с текущего момента времени.
//  hUserData - пользовательские данные для передачи в функцию обратного вызова.
//  callback - функция обратного вызова. Данная функция должна быть вызвана после записи 
//		файла (полностью или частично из-за возникновения ошибки) и его закрытия. При ошибке при
//		создании файла данная функция вызывается с аргументом lpFileName равным NULL.
//  phFileWritingId - адрес для записи идентификатора записываемого файла.
//  return value - 0 при успешном запуске процедуры записи файла. Ненулевое значение говорит об ошибке.

FW_RESULT_TYPE StartFileWriting(FileWritingDest fwDest, 
								int nStream, 
								FileWritingFormat fwFormat,
								int nDurationBefore,
								int nDurationAfter,
								FW_HANDLE_TYPE hUserData,
								FW_CALLBACK_TYPE callback,
								FW_HANDLE_TYPE * phFileWritingId);


// Функция принудительного завершения записи файла и закрытия идентификатора записываемого файла.
// Вызывается для каждого идентификатора записываемого файла, полученного после успешного 
// выполнения функции StartFileWriting, вне зависимости от того, была ли уже вызвана
// функция обратного вызова.
//  hFileWritingId - закрываемый идентификатор записываемого файла.

FW_RESULT_TYPE StopFileWriting(FW_HANDLE_TYPE hFileWritingId);
