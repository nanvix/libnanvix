// Copyright(c) The Maintainers of Nanvix.
// Licensed under the MIT License.

//==================================================================================================
// Configuration
//==================================================================================================

#![deny(clippy::all)]
#![forbid(clippy::large_stack_frames)]
#![forbid(clippy::large_stack_arrays)]
#![feature(panic_info_message)]
#![no_std]

//==================================================================================================
// Modules
//==================================================================================================

mod panic;
pub mod logging;

//==================================================================================================
// Exports
//==================================================================================================

pub use kcall::debug;

#[macro_export]
macro_rules! log{
	( $($arg:tt)* ) => ({
		use core::fmt::Write;
		use $crate::logging::Logger;
		let _ = writeln!(&mut Logger, $($arg)*);
	})
}
